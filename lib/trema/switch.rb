require 'pio'

module Trema
  # OpenFlow switch.
  class Switch
    attr_reader :error_message

    class InitError < StandardError; end

    include Pio

    OPENFLOW_HEADER_LENGTH = 8

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
      raise 'Switch is not initialized.' unless @features_reply
      @features_reply.datapath_id
    end
    alias dpid datapath_id

    def write(message)
      @socket.write message.to_binary
    end

    def read
      OpenFlow.read read_openflow_binary
    end

    private

    def exchange_hello_messages
      write Hello.new
      expect_receiving Hello
    end

    def exchange_echo_messages
      write Echo::Request.new
      expect_receiving Echo::Reply
    end

    def exchange_features_messages
      write Features::Request.new
      @features_reply = expect_receiving(Features::Reply)
    end

    # rubocop:disable MethodLength
    def expect_receiving(expected_message_klass)
      loop do
        message = read
        case message
        when expected_message_klass
          return message
        when Echo::Request
          write Echo::Reply.new(xid: message.xid)
        when PacketIn, PortStatus # , FlowRemoved (not implemented yet)
          return
        when OpenFlow10::Error::HelloFailed, OpenFlow13::Error::HelloFailed
          @error_message = message
          fail InitError, message.description
        else
          raise "Failed to receive #{expected_message_klass} message"
        end
      end
    end
    # rubocop:enable MethodLength

    def read_openflow_binary
      header_binary = drain(OPENFLOW_HEADER_LENGTH)
      header = OpenFlowHeaderParser.read(header_binary)
      body_binary = drain(header.message_length - OPENFLOW_HEADER_LENGTH)
      raise if (header_binary + body_binary).length != header.message_length
      header_binary + body_binary
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
