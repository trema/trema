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


require "trema/set-transport-port"


module Trema
  #
  # An action to modify the destination TCP/UDP port of a packet.
  #
  class SetTransportDstPort < SetTransportPort
    #
    # Creates an action to modify the destination TCP/UDP port of a packet.
    #
    # @example
    #   SetTransportDstPort.new( 5555 )
    #
    # @param [Integer] port_number
    #   the destination TCP/UDP port number. Any numeric 16-bit value.
    #
    # @raise [ArgumentError] if port_number is not an unsigned 16-bit Integer.
    # @raise [TypeError] if port_number is not an Integer.
    #
    def initialize port_number
      super port_number
    end
  end


  ActionSetTpDst = SetTransportDstPort
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
