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
    arrHops = path.hops()
    arrHops.each do |hop|
      info "\npath.match:" + path.match().inspect + "\npath.priority:" + path.priority().inspect + "\npath.idle:" + path.idle_timeout().inspect + "\npath.hard_timeout:" + path.hard_timeout().inspect + "\ndatapath_id:" + hop.datapath_id().inspect + "\n:in_port:" + hop.in_port().inspect + "\n:out_port:" + hop.out_port().inspect + "\n:actions:" + hop.actions().inspect
    end
  end
  
  def flow_manager_teardown_reply(reason, path)
    arrHops = path.hops()
    info "datapath_id:" + arrHops[0].datapath_id().inspect
    oneshot_timer_event(:shutdown, 5)
  end
  
  def switch_ready datapath_id
 	  info "***Hello %#x from #{ ARGV[ 0 ] }!" % datapath_id
  end
  
  def test
    #Array actions = [SendOutPort.new(1)]
  	hop11 = Hop.new(0x1,1,2)
    hop12 = Hop.new(0x2,3,4)
  	match = Match.new()
    path1 = Path.new(match, options={:priority => 10000, :idle_timeout=>13, :hard_timeout=>31})
    path1 << hop11
    path1.append_hop(hop12)
    path1.setup(self)

    hop21 = Hop.new(0x1,5,6)
    hop22 = Hop.new(0x2,7,8)
  	match2 = Match.new(:nw_src => "192.168.0.1/32")
    path2 = Path.new(match2, options={:priority => 20000, :idle_timeout=>12, :hard_timeout=>32}) 
    path2 << hop21
    path2 << hop22
    path2.setup(self)

  	hop31 = Hop.new(0x1,9,10)
    hop32 = Hop.new(0x2,11,12)
  	match3 = Match.new(:nw_src => "192.168.0.2/32")
    path3 = Path.new(match3, options={:priority => 30000, :idle_timeout=>11, :hard_timeout=>33})
    path3.append_hop(hop31)
    path3.append_hop(hop32)
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
