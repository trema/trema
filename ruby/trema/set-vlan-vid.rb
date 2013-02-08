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


module Trema
  #
  # An action to modify the VLAN ID of a packet.
  #
  class SetVlanVid < Action
    # @return [Fixnum] the value of attribute {#vlan_id}
    attr_reader :vlan_id


    #
    # Creates an action to modify the VLAN ID of a packet. The VLAN ID
    # is 16-bits long but the actual VID (VLAN Identifier) of the IEEE
    # 802.1Q frame is 12-bits.
    #
    # @example
    #   ActionSetVlanVid.new( vlan_id )
    #
    # @param [Integer] vlan_id
    #   the VLAN ID to set to. Only the lower 12-bits are used.
    #
    # @raise [ArgumentError] if vlan_id not within 1 and 4095 inclusive.
    # @raise [TypeError] if vlan_id is not an Integer.
    #
    def initialize vlan_id
      unless vlan_id.is_a?( Integer )
        raise TypeError, "VLAN ID argument must be an Integer"
      end
      unless ( vlan_id >= 1 and vlan_id <= 4095 )
        raise ArgumentError, "Valid VLAN ID values between 1 to 4095 inclusive"
      end
      @vlan_id = vlan_id
    end
  end


  ActionSetVlanVid = SetVlanVid
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
