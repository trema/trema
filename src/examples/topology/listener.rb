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


class TopologyListener < Controller
  include Topology
  
  def start
    @nodes = {}
    @edges = {}
    super()
  end

  periodic_timer_event :show_topology, 5

  def show_topology
    info %Q(graph "#{Time.now.strftime "%Y-%m-%dT%H:%M:%S" }" {)
    @nodes.keys.each do | dpid |
      info %q(  "%#x") % dpid
    end
    @edges.keys.each do | dpid_pair |
      info %q(  "%#x" -- "%#x") % dpid_pair
    end
    info "}"
  end

  oneshot_timer_event :on_start, 0

  def on_start
    get_all_switch_status do |sw_stats|
      sw_stats.each { |sw_stat|
        if sw_stat[:up]
          switch_status_up sw_stat[ :dpid ]
        else
          switch_status_down sw_stat[ :dpid ]
        end
      }
    end

    get_all_link_status do |link_stats|
      link_stats.each( & method(:link_status_updated) )
    end
  end


  def on_link_up_or_updated link
    dpid_pair = [ link.from_dpid, link.to_dpid ].minmax
    @edges[ dpid_pair ] = dpid_pair
  end


  def on_link_down link
    dpid_pair = [ link.from_dpid, link.to_dpid ].minmax
    @edges.delete dpid_pair
  end


  ###########################
  # Topology event handlers #
  ###########################


  def link_status_updated link_stat
    link = Link.new link_stat

    if link.up?
      on_link_up_or_updated link
    else
      on_link_down link
    end
  end


  def switch_status_up dpid
    @nodes[ dpid ] = dpid
  end


  def switch_status_down dpid
    @nodes.delete dpid
  end
end