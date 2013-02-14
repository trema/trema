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
    #arrAction1 = arrHops[0].actions()
    #info "arrAction1[0].port_number():" + arrAction1[0].port_number().inspect
    info "arrHops[1].datapath_id:" + arrHops[1].datapath_id().inspect
    info "arrHops[1].in_port:" + arrHops[1].in_port().inspect
    info "arrHops[1].out_port:" + arrHops[1].out_port().inspect
    info "arrHops[1].actions:" + arrHops[1].actions().inspect
  end
  
  def flow_manager_teardown_reply(reason, path)
    oneshot_timer_event(:shutdown, 1)
  end

  def features_reply datapath_id, message
    info "***features_replay***"
    p message.ports
  end

  def stats_reply datapath_id, message
    info "***stats_replay***"
    puts message.stats
  end 
  
  def switch_ready datapath_id
 	  info "***Hello %#x from #{ ARGV[ 0 ] }!" % datapath_id
  end
  
  def test
    Array actions = [SendOutPort.new(1)]
  	hop = Hop.new(0x1,2,1)
    hop2 = Hop.new(0x2,2,1)
  	match = Match.new()
    path = Path.new(match, options={:idle_timeout=>15, :hard_timeout=>30})

    path << hop
    path.append_hop(hop2)

    path.setup(self)

    info "***exit switch ready FlowManagerController"
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
