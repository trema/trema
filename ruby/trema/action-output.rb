#
# Copyright (C) 2008-2012 NEC Corporation
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
  class ActionOutput < Action
    attr_reader :max_len
    attr_reader :port


    #
    # Creates an action to output a packet to a port.
    #
    # @example
    #   ActionOutput.new( 1 )
    #   ActionOutput.new( :port => 1, :max_len => 256 )
    #   ActionOutput.new( :port => 1 )
    #   ActionOutput.new( :port => 1, :max_len => 256 )
    #
    # @param [Hash] options
    #   the options hash to create this action class instance with.
    #
    # @option options [Number] :port
    #   port number an index into switch's physical port list. There are also
    #   fake output ports. For example a port number set to +OFPP_FLOOD+ would
    #   output packets to all physical ports except input port and ports
    #   disabled by STP.
    #
    # @option options [Number] :max_len
    #   the maximum number of bytes from a packet to send to controller when port
    #   is set to +OFPP_CONTROLLER+. A zero length means no bytes of the packet
    #   should be sent. It defaults to 64K.
    #
    # @raise [ArgumentError] if port is not an unsigned 16-bit integer.
    # @raise [ArgumentError] if max_len is not an unsigned 16-bit integer.
    #
    def initialize options
      case options
        when Hash
          @port = options[ :port ]
          @max_len = options[ :max_len ]
          if @port.nil?
            raise ArgumentError, ":port is a mandatory option"
          end
          if not @port.unsigned_16bit?
            raise ArgumentError, "Port must be an unsigned 16-bit integer"
          end
          if @max_len
            if not @max_len.unsigned_16bit?
              raise ArgumentError, ":max_len must be an unsigned 16-bit integer"
            end
          else
            @max_len = 2 ** 16 - 1
          end
        when Integer
          if not options.unsigned_16bit?
            raise ArgumentError, "Port must be an unsigned 16-bit integer"
          end
          @port = options
          @max_len = 2 ** 16 - 1
        else
          raise "Invalid option"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
