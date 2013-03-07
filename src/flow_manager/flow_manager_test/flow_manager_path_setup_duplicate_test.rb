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
    oneshot_timer_event(:shutdown, 1)
  end 
  
  def switch_ready datapath_id
 	info "***Hello %#x from #{ ARGV[ 0 ] }!" % datapath_id
  end
  
  def test
  
    Array actions = [StripVlanHeader.new]
  	hop = Hop.new(0x1,1,2, actions)
  	match = Match.new(:in_port => 1)
    path = Path.new(match, options={:idle_timeout=>10})
    Flow_manager.append_hop_to_path(path,hop);
    path.setup(self)
    path.setup(self)

  end

  def shutdown
    self.shutdown!
  end
end
