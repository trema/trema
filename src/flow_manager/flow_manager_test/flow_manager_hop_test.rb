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
require 'trema/flow-manager'

class FlowManagerController < Controller
  include Trema::FlowManager
  oneshot_timer_event(:test, 3)
  
  def test
  	
    hop1 = Hop.new(0x1,1,2)
    hop2 = Hop.new(0x2,2,3)

    info "hop1.datapath_id:" + hop1.datapath_id().inspect
    info "hop1.in_port:" + hop1.in_port().inspect
    info "hop1.out_port:" + hop1.out_port().inspect
    info "hop2.datapath_id:" + hop2.datapath_id().inspect
    info "hop2.in_port:" + hop2.in_port().inspect
    info "hop2.out_port:" + hop2.out_port().inspect

    Array actions = Array.new([SendOutPort.new(:port_number => 1, :max_len => 256), 
                      SetEthSrcAddr.new("11:22:33:44:55:66"),
                      SetEthDstAddr.new("11:22:33:44:55:66"),
                      SetIpSrcAddr.new("192.168.1.1"),
                      SetIpDstAddr.new("192.168.1.1"),
                      SetIpTos.new(32),
                      SetTransportSrcPort.new( 5555 ),
                      SetTransportDstPort.new( 5555 ),
                      ActionSetVlanVid.new( 1 ),
                      SetVlanPriority.new( 7 ),
                      StripVlanHeader.new,
                      VendorAction.new( 0x00004cff, Array["test", "test2"] )])
    hop3 = Hop.new(0x3,3,4, actions)
    info "hop3.actions:" + hop3.actions().inspect
    info "hop3.actions[0].max_len:" + hop3.actions[0].max_len.inspect
    info "hop3.actions[0].port_number:" + hop3.actions[0].port_number.inspect
    info "hop3.actions[1].mac_address:" + hop3.actions[1].mac_address.inspect
    info "hop3.actions[2].mac_address:" + hop3.actions[2].mac_address.inspect
    info "hop3.actions[3].ip_address:" + hop3.actions[3].ip_address.inspect
    info "hop3.actions[4].ip_address:" + hop3.actions[4].ip_address.inspect
    info "hop3.actions[5].type_of_service:" + hop3.actions[5].type_of_service.inspect
    info "hop3.actions[6].port_number:" + hop3.actions[6].port_number.inspect
    info "hop3.actions[7].port_number:" + hop3.actions[7].port_number.inspect
    info "hop3.actions[8].vlan_id:" + hop3.actions[8].vlan_id.inspect
    info "hop3.actions[9].vlan_priority:" + hop3.actions[9].vlan_priority.inspect
    info "hop3.actions[11].vendor_id:" + hop3.actions[11].vendor_id.inspect


    oneshot_timer_event(:shutdown, 1)
  end

  def shutdown
    self.shutdown!
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
