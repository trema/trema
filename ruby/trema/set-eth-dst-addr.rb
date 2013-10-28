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


require "trema/set-eth-addr"


module Trema
  #
  # An action to modify the destination Ethernet address of a packet.
  #
  class SetEthDstAddr < SetEthAddr
    #
    # Creates an action to modify the destination Ethernet address of a packet.
    #
    # @overload initialize(mac_address)
    #
    # @example
    #   SetEthDstAddr.new("11:22:33:44:55:66")
    #   SetEthDstAddr.new(0x112233445566)
    #   SetEthDstAddr.new(Mac.new("11:22:33:44:55:66"))
    #   SetEthDstAddr.new(Mac.new(0x112233445566))
    #
    # @param mac_address [#to_str, #to_int]
    #   the Ethernet address to create this action with.
    #
  end


  ActionSetDlDst = SetEthDstAddr
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
