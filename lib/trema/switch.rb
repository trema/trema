require 'pio'

module Trema
  # OpenFlow switch.
  class Switch
    include Pio

    OPENFLOW_HEADER_LENGTH = 8

    OPENFLOW_MESSAGE_PARSER = {
      OpenFlow::HELLO => Hello,
      OpenFlow::ECHO_REQUEST => Echo::Request,
      OpenFlow::ECHO_REPLY => Echo::Reply,
      OpenFlow::FEATURES_REPLY => Features::Reply,
      OpenFlow::PACKET_IN => PacketIn,
      OpenFlow::PORT_STATUS => PortStatus
    }

    def initialize(socket)
      @socket = socket
    end

    def init
      exchange_hello_messages
      exchange_echo_messages
      exchange_features_messages
      self
    end

    def datapath_id
      fail 'Switch is not initialized.' unless @features_reply
      @features_reply.datapath_id
    end
    alias_method :dpid, :datapath_id

    def write(message)
      @socket.write message.to_binary
    end

    def read
      message_type, binary = read_openflow_binary
      OPENFLOW_MESSAGE_PARSER.fetch(message_type).read(binary)
    rescue KeyError
      raise "Unknown OpenFlow message (message_type = #{message_type})."
    end

    private

    def exchange_hello_messages
      fail 'Failed to exchange Hello messages' unless read.is_a?(Hello)
      write Hello.new
    end

    def exchange_echo_messages
      write Echo::Request.new
      fail 'Failed to exchange Echo messages' unless read.is_a?(Echo::Reply)
    end

    def exchange_features_messages
      write Features::Request.new
      @features_reply = read
      return if @features_reply.is_a?(Features::Reply)
      fail 'Failed to exchange Features messages'
    end

    def read_openflow_binary
      header_binary = drain(OPENFLOW_HEADER_LENGTH)
      header = OpenFlow::OpenFlowHeader.read(header_binary)
      body_binary = drain(header.message_length - OPENFLOW_HEADER_LENGTH)
      fail if (header_binary + body_binary).length != header.message_length
      [header.message_type, header_binary + body_binary]
    end

    def drain(length)
      buffer = ''
      loop do
        buffer += @socket.readpartial(length - buffer.length)
        break if buffer.length == length
      end
      buffer
    end
  end
end
