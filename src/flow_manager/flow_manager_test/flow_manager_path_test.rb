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
	  match = Match.new(
      :in_port => 1,
      :dl_src => "00:00:00:00:00:01",
      :dl_dst => "00:00:00:00:00:02",
      :dl_vlan => 65535,
      :dl_vlan_pcp => 0,
      :dl_type => 0x800,
      :nw_tos => 0,
      :nw_proto => 17,
      :nw_src => "192.168.0.1",
      :nw_dst => "192.168.0.2",
      :tp_src => 10,
      :tp_dst => 20
    )
    path = Path.new(match)
    path2 = Path.new(match, options={:idle_timeout=>15, :hard_timeout=>30, :priority=>60000})

    info "path.priority1:" + path.priority().inspect
    info "path.idle_timeout1:" + path.idle_timeout().inspect
    info "path.hard_timeout1:" + path.hard_timeout().inspect
    info "path.match1:" + path.match().inspect
    info "path2.priority1:" + path2.priority().inspect
    info "path2.idle_timeout1:" + path2.idle_timeout().inspect
    info "path2.hard_timeout1:" + path2.hard_timeout().inspect
    info "path2.match1:" + path2.match().inspect

    Array actions = [SendOutPort.new(1)]
    hop1 = Hop.new(0x2,2,3,actions)
    hop2 = Hop.new(0x1,1,2)
   
    path << hop1
    path.append_hop(hop2)

    arrHops = path.hops()
    info "arrHops[0].datapath_id1:" + arrHops[0].datapath_id().inspect
    info "arrHops[0].in_port1:" + arrHops[0].in_port().inspect
    info "arrHops[0].out_port1:" + arrHops[0].out_port().inspect
    info "arrHops[0].actions1:" + arrHops[0].actions().inspect
    info "arrHops[1].datapath_id1:" + arrHops[1].datapath_id().inspect
    info "arrHops[1].in_port1:" + arrHops[1].in_port().inspect
    info "arrHops[1].out_port1:" + arrHops[1].out_port().inspect
    info "arrHops[1].actions1:" + arrHops[1].actions().inspect

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
