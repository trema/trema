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
  # An action to modify the VLAN priority of a packet.
  #
  class SetVlanPriority < Action
    # @return [Fixnum] the value of attribute {#vlan_priority}
    attr_reader :vlan_priority


    #
    # Creates an action to modify the VLAN priority of a
    # packet. Priority bits can be used to prioritize different
    # classes of traffic. Valid values are between 0 (lowest) and 7
    # (highest).
    #
    # @example
    #   SetVlanPriority.new( 7 )
    #
    # @param [Integer] vlan_priority
    #   the VLAN priority to set to.
    #
    # @raise [ArgumentError] if vlan_priority is not within 0 and 7 inclusive.
    # @raise [TypeError] if vlan_priority is not an Integer.
    #
    def initialize vlan_priority
      unless vlan_priority.is_a?( Integer )
        raise TypeError, "VLAN priority must be an unsigned 8-bit Integer"
      end
      unless ( vlan_priority >= 0 and vlan_priority <= 7 )
        raise ArgumentError, "Valid VLAN priority values are 0 to 7 inclusive"
      end
      @vlan_priority = vlan_priority
    end
  end


  ActionSetVlanPcp = SetVlanPriority
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
