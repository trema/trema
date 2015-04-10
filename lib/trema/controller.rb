require 'logger'
require 'phut'
require 'pio'
require 'socket'
require 'trema/command'
require 'trema/logger'
require 'trema/monkey_patch/integer'
require 'trema/switch'

module Trema
  # The base class of Trema controller. Subclass and override handlers
  # to implement a custom OpenFlow controller.
  #
  # rubocop:disable ClassLength
  class Controller
    # Pio::FlowMod.new argument
    class FlowModAddOption
      def initialize(user_options)
        @user_options = user_options
      end

      def to_hash
        {
          command: :add,
          transaction_id: rand(0xffffffff),
          hard_timeout: @user_options[:hard_timeout] || 0,
          buffer_id: @user_options[:buffer_id] || 0xffffffff,
          match: @user_options.fetch(:match),
          actions: @user_options[:actions] || []
        }
      end
    end

    # Pio::FlowMod.new argument
    class FlowModDeleteOption
      def initialize(user_options)
        @user_options = user_options
      end

      def to_hash
        {
          command: :delete,
          transaction_id: rand(0xffffffff),
          buffer_id: @user_options[:buffer_id] || 0xffffffff,
          match: @user_options.fetch(:match),
          out_port: @user_options[:out_port] || 0xffff
        }
      end
    end

    # Pio::PacketOut.new argument
    class PacketOutOption
      def initialize(user_options)
        @user_options = user_options
      end

      # rubocop:disable MethodLength
      def to_hash
        if @user_options[:packet_in]
          {
            transaction_id: rand(0xffffffff),
            buffer_id: 0xffffffff,
            actions: @user_options[:actions],
            in_port: @user_options.fetch(:packet_in).in_port,
            raw_data: @user_options.fetch(:packet_in).raw_data
          }
        else
          {
            transaction_id: rand(0xffffffff),
            buffer_id: 0xffffffff,
            actions: @user_options[:actions],
            raw_data: @user_options.fetch(:raw_data)
          }
        end
      end
      # rubocop:enable MethodLength
    end

    class << self
      attr_accessor :logging_level
    end

    include Pio

    SWITCH = {}
    DEFAULT_TCP_PORT = 6633

    # @return [Logger]
    attr_reader :logger

    def self.timer_event(handler, options)
      @timer_handlers ||= {}
      @timer_handlers[handler] = options.fetch(:interval)
    end

    def self.timer_handlers
      @timer_handlers || {}
    end

    # @private
    # rubocop:disable TrivialAccessors
    def self.inherited(subclass)
      @controller_klass = subclass
    end
    # rubocop:enable TrivialAccessors

    # @private
    def self.create
      @controller_klass.new
    end

    # @private
    def initialize
      @threads = []
      @logger = Logger.new(name)
      @logger.level = Controller.logging_level
    end

    # @private
    # Starts this controller. Usually you do not need to invoke
    # explicitly, because this is called implicitly by "trema run"
    # command.
    def run(args)
      drb_socket_file =
        File.expand_path(File.join(Phut.socket_dir, "#{name}.ctl"))
      @drb = DRb::DRbServer.new 'drbunix:' + drb_socket_file, self
      maybe_send_handler :start, args
      socket = TCPServer.open('<any>', DEFAULT_TCP_PORT)
      start_timers
      loop { start_switch_thread(socket.accept) }
    end

    # @private
    def name
      self.class.name
    end

    def stop
      @drb.stop_service if @drb
      @threads.map(&:kill)
    end

    # @!group OpenFlow Message

    def send_flow_mod_add(datapath_id, options)
      flow_mod = FlowMod.new(FlowModAddOption.new(options).to_hash)
      send_message datapath_id, flow_mod
    end

    def send_flow_mod_delete(datapath_id, options)
      flow_mod = FlowMod.new(FlowModDeleteOption.new(options).to_hash)
      send_message datapath_id, flow_mod
    end

    def send_packet_out(datapath_id, options)
      packet_out = PacketOut.new(PacketOutOption.new(options).to_hash)
      send_message datapath_id, packet_out
    end

    def send_message(datapath_id, message)
      SWITCH.fetch(datapath_id).write message
    rescue KeyError, Errno::ECONNRESET, Errno::EPIPE
      logger.debug "Switch #{datapath_id} is disconnected."
    end

    # @!endgroup

    # @!group Handlers

    # @private Just a placeholder for YARD.
    def self._handler(_name)
      # Noop.
    end

    # @!method start(argv)
    #
    # Start event handler. Override this to implement a custom
    # handler.
    _handler :start

    # @!method switch_ready(datapath_id)
    #
    # Switch Ready event handler. Override this to implement a custom
    # handler.
    _handler :switch_ready

    # The default handler for echo request messages.
    # Override this to implement a custom handler.
    def echo_request(datapath_id, message)
      echo_reply = Echo::Reply.new(message.xid)
      send_message datapath_id, echo_reply
    end

    # @!method packet_in(datapath_id, message)
    #
    # Packet In message handler. Override this to implement a custom
    # handler.
    _handler :packet_in

    # @!endgroup

    private

    def start_timers
      self.class.timer_handlers.each do |handler, interval|
        th = Thread.start(handler, interval) do |method, sec|
          loop do
            send_handler method
            sleep sec
          end
        end
        th.abort_on_exception = true
        @threads << th
      end
    end

    def start_switch_thread(socket)
      th = Thread.start(socket) do |sock|
        switch = create_and_register_new_switch(sock)
        start_switch_main switch.datapath_id
      end
      th.abort_on_exception = true
      @threads << th
    end

    def start_switch_main(datapath_id)
      maybe_send_handler :switch_ready, datapath_id
      loop { handle_openflow_message datapath_id }
    rescue EOFError, IOError
      unregister_switch datapath_id
    end

    def create_and_register_new_switch(socket)
      switch = Switch.new(socket).init
      SWITCH[switch.datapath_id] = switch
    end

    def unregister_switch(datapath_id)
      SWITCH.delete datapath_id
      maybe_send_handler :switch_disconnected, datapath_id
    end

    # rubocop:disable MethodLength
    # rubocop:disable AbcSize
    # rubocop:disable CyclomaticComplexity
    def handle_openflow_message(datapath_id)
      begin
        message = SWITCH.fetch(datapath_id).read
      rescue KeyError
        logger.debug "Switch #{datapath_id} is disconnected."
      end

      case message
      when Echo::Request
        maybe_send_handler :echo_request, datapath_id, message
      when Features::Reply
        maybe_send_handler :features_reply, datapath_id, message
      when PacketIn
        message.datapath_id = datapath_id
        maybe_send_handler :packet_in, datapath_id, message
      when PortStatus
        message.datapath_id = datapath_id
        case message.reason
        when :add
          maybe_send_handler :port_add, datapath_id, message
        when :delete
          maybe_send_handler :port_delete, datapath_id, message
        when :modify
          maybe_send_handler :port_modify, datapath_id, message
        else
          fail "Invalid Port Status message: #{message.inspect}"
        end
      else
        fail "Unknown OpenFlow message: #{message.inspect}"
      end
    end
    # rubocop:enable MethodLength
    # rubocop:enable AbcSize
    # rubocop:enable CyclomaticComplexity

    def send_handler(handler, *args)
      @handler_mutex ||= Mutex.new
      @handler_mutex.synchronize { __send__(handler, *args) }
    end

    def maybe_send_handler(handler, *args)
      return unless respond_to?(handler)
      send_handler(handler, *args)
    end
  end
  # rubocop:enable ClassLength
end
