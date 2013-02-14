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
  
  def flow_manager_setup_reply(status, path)
    info "path.priority:" + path.priority().inspect
    info "path.idle:" + path.idle_timeout().inspect
    info "path.hard_timeout:" + path.hard_timeout().inspect
    info "path.match:" + path.match().inspect
    arrHops = path.hops()
    info "arrHops[0].datapath_id:" + arrHops[0].datapath_id().inspect
    info "arrHops[0].in_port:" + arrHops[0].in_port().inspect
    info "arrHops[0].out_port:" + arrHops[0].out_port().inspect
    info "arrHops[1].datapath_id:" + arrHops[1].datapath_id().inspect
    info "arrHops[1].in_port:" + arrHops[1].in_port().inspect
    info "arrHops[1].out_port:" + arrHops[1].out_port().inspect
    info "arrHops[1].actions:" + arrHops[1].actions().inspect
  end
  
  def flow_manager_teardown_reply(reason, path)
    oneshot_timer_event(:shutdown, 1)
  end
  
  def switch_ready datapath_id
 	  info "***Hello %#x from #{ ARGV[ 0 ] }!" % datapath_id
  end
  
  def test
    Array actions = [SendOutPort.new(1)]
  	hop11 = Hop.new(0x1,2,1)
    hop12 = Hop.new(0x2,2,1)
  	match = Match.new()
    path1 = Path.new(match, options={:idle_timeout=>10, :hard_timeout=>30})
    path1 << hop11
    path1.append_hop(hop12)

    hop21 = Hop.new(0x1,1,2)
    hop22 = Hop.new(0x2,1,2)
  	match2 = Match.new(:nw_src => "192.168.0.1/32")
    path2 = Path.new(match2, options={:idle_timeout=>10, :hard_timeout=>30}) 
    path2 << hop21
    path2.append_hop(hop22)

  	hop31 = Hop.new(0x1,2,1)
    hop32 = Hop.new(0x2,2,1)
  	match3 = Match.new(:nw_src => "192.168.0.2/32")
    path3 = Path.new(match3, options={:priority => 60000, :idle_timeout=>10, :hard_timeout=>30})
    path3 << hop31
    path3.append_hop(hop32)

    path1.setup(self)
    path2.setup(self)
    path3.setup(self)
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
