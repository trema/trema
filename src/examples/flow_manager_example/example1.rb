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

#Please include following module Trema::FlowManager
require 'trema/flow-manager'

class FlowManagerController < Controller
  include Trema::FlowManager
  oneshot_timer_event(:test, 5)
  
  #This event is called when you recieve the result of setup path.
  def flow_manager_setup_reply(status, path)
  	info "*****flow_manager_setup_reply*****" 
    info status
    dump_path(path)
  end
  
  #This event is called when you recieve teardown path.
  def flow_manager_teardown_reply(reason, path)
  	info "*****flow_manager_teardown_reply*****" 
  	info reason
    dump_path(path)

    oneshot_timer_event(:shutdown, 1)
  end 

  def dump_path path
    info "path.priority:" + path.priority().inspect
    info "path.idle_timeout:" + path.idle_timeout().inspect
    info "path.hard_timeout:" + path.hard_timeout().inspect
    info "path.match:" + path.match().inspect
    Array hops = path.hops
    info "number of hops:" + hops.size().inspect
  end
  
  def test
    match = Match.new()

    #Instantiate some hops which represent swiches.
    #Hop.new(Integer datapath id, Integer in_port, Integer out_port, Array actions=[])
  	hop = Hop.new(0x1,1,2)
    hop2 = Hop.new(0x2,2,1)
	
    #Instantiate a path.
    #Path.new(Match match, Hash options={:priority, :idle_timeout, :hard_timeout})
    path = Path.new(match, options={:priority=>65535, :idle_timeout=>5, :hard_timeout=>20})
    
    #Set hops to the path.
    #Flow_manager.sppend_hop_to_path(Path path, Hop hop) 
    Flow_manager.append_hop_to_path(path, hop)
    Flow_manager.append_hop_to_path(path, hop2)

    info("***** Setting up a path *****")

    #You can access properties by following getters.
    info "path.priority:" + path.priority().inspect
    info "path.idle_timeout:" + path.idle_timeout().inspect
    info "path.hard_timeout:" + path.hard_timeout().inspect
    info "path.match:" + path.match().inspect
    Array hops = path.hops
    info "number of hops:" + hops.size().inspect
    info "Hop0x1 datapath_id:" + hops[0].datapath_id.inspect
    info "Hop0x1 in_port:" + hops[0].in_port.inspect
    info "Hop0x1 out_port:" + hops[0].out_port.inspect
    info "Hop0x2 datapath_id:" + hops[1].datapath_id.inspect
    info "Hop0x2 in_port:" + hops[1].in_port.inspect
    info "Hop0x2 out_port:" + hops[1].out_port.inspect

    #Start setup the path to openflow switches.
    Flow_manager.setup(path, self)

  end

  def shutdown
    self.shutdown!
  end
end
