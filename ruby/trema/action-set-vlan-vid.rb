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


module Trema
  #
  # An action to modify the VLAN id of a packet.
  #
  class ActionSetVlanVid < Action
    attr_reader :vlan_vid


    #
    # An action to modify the VLAN id of a packet. The VLAN id is
    # 16-bits long but the actual VID (VLAN Identifier) of the IEEE
    # 802.1Q frame is 12-bits.
    #
    # @example
    #   ActionSetVlanVid.new( vlan_vid )
    #
    # @param [Integer] vlan_vid
    #   the VLAN id to set to. Only the lower 12-bits are used.
    #
    # @raise [ArgumentError] if vlan_vid argument is not supplied.
    # @raise [ArgumentError] if vlan_vid not within 1 and 4096 inclusive.
    # @raise [TypeError] if vlan_vid is not an Integer.
    #
    def initialize vlan_vid
      if not vlan_vid.is_a?( Integer )
        raise TypeError, "VLAN id argument must be an Integer"
      end
      if not ( vlan_vid >= 1 and vlan_vid <= 4096 )
        raise ArgumentError, "Valid VLAN id values between 1 to 4096 inclusive"
      end
      @vlan_vid = vlan_vid
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
