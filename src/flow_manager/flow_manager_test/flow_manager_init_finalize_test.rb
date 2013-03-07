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
    info "*** flow_manager_setup_reply" 
  end
  
  def flow_manager_teardown_reply(reason, path)
  	info "***flow_manager_teardown_reply" 
    oneshot_timer_event(:shutdown, 1)
  end 
  
  def switch_ready datapath_id
 	info "***Hello %#x from #{ ARGV[ 0 ] }!" % datapath_id
  end
  
  def test

    info Flow_manager.initialize().to_s
    info Flow_manager.initialize().to_s
    info Flow_manager.initialize().to_s

    Array actions = [SendOutPort.new(1)]
  	hop = Hop.new(0x1,1,2,actions)
    hop2 = Hop.new(0x2,2,3)
  	match = Match.new()
    path = Path.new(match, options={:idle_timeout=>10, :hard_timeout=>30})

    path << hop
    path.append_hop(hop2)

    path.setup(self)

    info "***exit switch ready FlowManagerController"
  end

  def shutdown
    self.shutdown!
  end
end

