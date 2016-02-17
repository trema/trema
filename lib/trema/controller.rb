require 'logger'
require 'phut'
require 'pio'
require 'socket'
require 'trema/command'
require 'trema/logger'
require 'trema/monkey_patch/integer'

module Trema
  class NoControllerDefined < StandardError; end

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
          priority: @user_options[:priority] || 0,
          transaction_id: rand(0xffffffff),
          idle_timeout: @user_options[:idle_timeout] || 0,
          hard_timeout: @user_options[:hard_timeout] || 0,
          buffer_id: @user_options[:buffer_id] || 0xffffffff,
          match: @user_options.fetch(:match),
          actions: @user_options[:actions] || []
        }
      end
    end

    # Pio::FlowMod.new argument (OpenFlow 1.3)
    class FlowModAdd13Option
      def initialize(user_options)
        @user_options = user_options
      end

      # rubocop:disable MethodLength
      # rubocop:disable CyclomaticComplexity
      # rubocop:disable PerceivedComplexity
      def to_hash
        {
          command: :add,
          priority: @user_options[:priority] || 0,
          transaction_id: rand(0xffffffff),
          idle_timeout: @user_options[:idle_timeout] || 0,
          hard_timeout: @user_options[:hard_timeout] || 0,
          buffer_id: @user_options[:buffer_id] || 0xffffffff,
          match: @user_options.fetch(:match),
          table_id: @user_options[:table_id] || 0,
          flags: @user_options[:flags] || [],
          instructions: @user_options[:instructions] || []
        }
      end
      # rubocop:enable MethodLength
      # rubocop:enable CyclomaticComplexity
      # rubocop:enable PerceivedComplexity
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
          out_port: @user_options[:out_port] || 0xffffffff
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

    include Pio

    SWITCH = {} # rubocop:disable MutableConstant
    DEFAULT_TCP_PORT = 6653

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
    def self.inherited(subclass)
      @controller_klass = subclass
    end

    # @private
    def self.create(port_number = DEFAULT_TCP_PORT,
                    logging_level = ::Logger::INFO)
      unless @controller_klass
        raise NoControllerDefined, 'No controller class is defined.'
      end
      @controller_klass.new(port_number, logging_level)
    end

    # @private
    def initialize(port_number = DEFAULT_TCP_PORT,
                   logging_level = ::Logger::INFO)
      @port_number = port_number
      @threads = []
      @logger = Logger.new(name)
      @logger.level = logging_level
    end

    # @private
    # Starts this controller. Usually you do not need to invoke
    # explicitly, because this is called implicitly by "trema run"
    # command.
    def run(args)
      maybe_send_handler :start, args
      socket = TCPServer.open('<any>', @port_number)
      start_timers
      loop { start_switch_thread(socket.accept) }
    end

    def name
      self.class.name
    end

    def stop
      @threads.map(&:kill)
    end

    # @!group OpenFlow Message

    def send_flow_mod_add(datapath_id, options)
      flow_mod =
        case Pio::OpenFlow.version
        when 'OpenFlow10'
          FlowMod.new(FlowModAddOption.new(options).to_hash)
        when 'OpenFlow13'
          FlowMod.new(FlowModAdd13Option.new(options).to_hash)
        else
          raise "Unsupported OpenFlow version: #{Pio::OpenFlow.version}"
        end
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
      echo_reply = Echo::Reply.new(transaction_id: message.xid)
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
      switch = Switch.new(socket)
      switch.init
      SWITCH[switch.datapath_id] = switch
    rescue Switch::InitError
      error_message = switch.error_message
      case error_message
      when OpenFlow10::Error::HelloFailed, OpenFlow13::Error::HelloFailed
        maybe_send_handler :hello_failed, error_message
        raise $ERROR_INFO
      end
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
      when Echo::Reply
        maybe_send_handler :echo_reply, datapath_id, message
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
          raise "Invalid Port Status message: #{message.inspect}"
        end
      when Barrier::Reply
        maybe_send_handler :barrier_reply, datapath_id, message
      when DescriptionStats::Reply
        maybe_send_handler :description_stats_reply, datapath_id, message
      else
        raise "Unknown OpenFlow message: #{message.inspect}"
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
