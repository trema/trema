require 'pio'

module Trema
  # OpenFlow switch.
  class Switch
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
      fail 'Switch is not initialized.' unless @features_reply
      @features_reply.datapath_id
    end
    alias_method :dpid, :datapath_id

    def write(message)
      @socket.write message.to_binary
    end

    def read
      OpenFlow.read read_openflow_binary
    end

    private

    def exchange_hello_messages
      fail 'Failed to exchange Hello messages' unless read.is_a?(Hello)
      write Hello.new
    end

    def exchange_echo_messages
      write Echo::Request.new
      loop do
        message = read
        if message.is_a?(Echo::Reply)
          break
        else
          handle_early message 'Failed to exchange Echo messages'
        end
      end
    end

    def exchange_features_messages
      write Features::Request.new
      loop do
        message = read
        if message.is_a?(Features::Reply)
          @features_reply = message
          break
        else
          handle_early message 'Failed to exchange Features messages'
        end
      end
    end

    def handle_early(message, fail_message)
      case message
      when Echo::Request
        write Echo::Reply.new xid: message.xid
      when PacketIn, FlowRemoved, PortStatus
        return
      else
        fail fail_message
      end
    end

    def read_openflow_binary
      header_binary = drain(OPENFLOW_HEADER_LENGTH)
      header = OpenFlowHeaderParser.read(header_binary)
      body_binary = drain(header.message_length - OPENFLOW_HEADER_LENGTH)
      fail if (header_binary + body_binary).length != header.message_length
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
