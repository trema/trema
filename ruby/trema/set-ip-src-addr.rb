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


require "trema/set-ip-addr"


module Trema
  #
  # An action to modify the IPv4 source address of a packet.
  #
  class SetIpSrcAddr < SetIpAddr
    #
    # Creates an action to modify the IPv4 source address of a packet.
    #
    # @overload initialize(ip_address)
    #
    # @example
    #   SetIpSrcAddr.new("192.168.1.1")
    #
    # @param [String] ip_address
    #   the IPv4 address to create this action with.
    #
    # @raise [ArgumentError] if ip_address is an invalid IPv4 address.
    # @raise [TypeError] if ip_address is not a String.
    #
  end


  ActionSetNwSrc = SetIpSrcAddr
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
