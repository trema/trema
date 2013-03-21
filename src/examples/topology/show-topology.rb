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


class ShowTopology < Controller
  include Topology

  EXISTS = nil

  oneshot_timer_event :timed_out, 10
  oneshot_timer_event :on_start, 0
  def on_start
    get_all_link_status
  end


  def all_link_status link_status
    dpids = Hash.new
    links = Hash.new

    debug "topology: entries #{link_status.size}"

    link_status.each do | link_hash |
      link = Link.new( link_hash )
      if link.up? then
        dpids[link.from_dpid] = EXISTS
        dpids[link.to_dpid] = EXISTS

        dpid_pair = [link.from_dpid, link.to_dpid]

        links[ [dpid_pair.max, dpid_pair.min] ] = EXISTS
      else
        debug "link down"
      end
    end

    dpids.keys.each do | dpid |
      puts "vswitch {"
      puts %Q(  datapath_id "0x#{dpid.to_s(16)}")
      puts "}\n\n"
    end

    links.keys.each do | dpid0, dpid1 |
      puts %Q(link "0x#{dpid0.to_s(16)}", "0x#{dpid1.to_s(16)}")
    end

    shutdown!
  end


  def timed_out
    error "timed out."
    shutdown!
  end
end
