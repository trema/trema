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


class ShowSwitchStatus < Controller
  include Topology


  oneshot_timer_event :timed_out, 15
  oneshot_timer_event :on_start, 0
  def on_start
    get_all_switch_status
  end

  def all_switch_status sw_status
    puts "Switch status"
    sw_status.each do | sw_hash |
      sw = Topology::Switch.new( sw_hash )

      status_str = "unknown"
      if sw.up? then
        status_str = "up"
      else
        status_str = "down"
      end
      puts "  dpid : 0x#{sw.dpid.to_s(16)}, status : #{status_str}"
    end

    get_all_port_status
  end

  def all_port_status port_status
    puts "Port status"
    port_status.each do | port_hash |
      port = Port.new( port_hash )
      status_str = "unknown"
      if port.up? then
        status_str = "up"
      else
        status_str = "down"
      end
      
      puts "  dpid : 0x#{port.dpid.to_s(16)}, port : #{port.portno.to_s}(#{port.name}), status : #{status_str}, external : #{port.external?.to_s}"
    end

    shutdown!
  end


  def timed_out
    error "timed out."
    shutdown!
  end
end
