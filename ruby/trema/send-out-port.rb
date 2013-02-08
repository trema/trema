#
# Copyright (C) 2008-2013 NEC Corporation
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License, version 2, as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#


require "trema/action"
require "trema/monkey-patch/integer"


module Trema
  #
  # An action to output a packet to a port.
  #
  class SendOutPort < Action
    DEFAULT_MAX_LEN = 2 ** 16 - 1


    # @return [Fixnum] the value of attribute {#max_len}
    attr_reader :max_len
    # @return [Fixnum] the value of attribute {#port_number}
    attr_reader :port_number


    #
    # Creates an action to output a packet to a port.
    #
    # @example
    #   SendOutPort.new( 1 )
    #   SendOutPort.new( :port_number => 1 )
    #   SendOutPort.new( :port_number => OFPP_CONTROLLER, :max_len => 256 )
    #
    # @param [Integer|Hash] options
    #   the port number or the options hash to create this action class instance with.
    #
    # @option options [Number] :port_number
    #   port number an index into switch's physical port list. There are also
    #   fake output ports. For example a port number set to +OFPP_FLOOD+ would
    #   output packets to all physical ports except input port and ports
    #   disabled by STP.
    # @option options [Number] :max_len
    #   the maximum number of bytes from a packet to send to controller when port
    #   is set to +OFPP_CONTROLLER+. A zero length means no bytes of the packet
    #   should be sent. It defaults to 64K.
    #
    # @raise [ArgumentError] if port_number is not an unsigned 16-bit integer.
    # @raise [ArgumentError] if max_len is not an unsigned 16-bit integer.
    #
    def initialize options
      case options
        when Hash
          @port_number = options[ :port_number ] || options[ :port ]
          @max_len = options[ :max_len ] || DEFAULT_MAX_LEN
        when Integer
          @port_number = options
          @max_len = DEFAULT_MAX_LEN
        else
          raise "Invalid option"
      end
      check_port_number
      check_max_len
    end


    def to_s
      "#{ self.class.to_s }: port_number=#{ @port_number }, max_len=#{ @max_len }"
    end


    ############################################################################
    private
    ############################################################################


    def check_port_number
      if @port_number.nil?
        raise ArgumentError, "Port number is a mandatory option"
      end
      unless @port_number.unsigned_16bit?
        raise ArgumentError, "Port number must be an unsigned 16-bit integer"
      end
    end


    def check_max_len
      unless @max_len.unsigned_16bit?
        raise ArgumentError, "Max length must be an unsigned 16-bit integer"
      end
    end
  end


  ActionOutput = SendOutPort
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
