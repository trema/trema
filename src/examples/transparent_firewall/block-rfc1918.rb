# A sample transparent firewall, see README.md for more information.
#
# Copyright (C) 2014 Denis Ovsienko
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

class BlockRFC1918 < Controller
  PORTS = {
    outside: 1,
    inside: 2,
    inspect: 3
  }

  PREFIXES = ['10.0.0.0/8', '172.16.0.0/12', '192.168.0.0/16'].map do |each|
    Pio::IPv4Address.new each
  end

  def switch_ready(dpid)
    if @dpid
      info "#{dpid.to_hex}: ignored"
      return
    end
    @dpid = dpid
    info "#{@dpid.to_hex}: connected"
    start_loading
  end

  def switch_disconnected(dpid)
    return if @dpid != dpid
    info "#{@dpid.to_hex}: disconnected"
    @dpid = nil
  end

  def barrier_reply(dpid, message)
    return if dpid != @dpid
    info "#{@dpid.to_hex}: loading finished"
  end

  private

  def start_loading
    PREFIXES.each do |each|
      block_prefix_on_port(each, PORTS[:inside], 5000)
      block_prefix_on_port(each, PORTS[:outside], 4000)
    end
    install_postamble(1500)
    send_message(@dpid, BarrierRequest.new)
  end

  def block_prefix_on_port(prefix, port_number, priority)
    send_flow_mod_add(
      @dpid,
      priority: priority + 100,
      match: Match.new(in_port: port_number, dl_type: 0x0800, nw_src: prefix),
      actions: SendOutPort.new(PORTS[:inspect]))
    send_flow_mod_add(
      @dpid,
      priority: priority,
      match: Match.new(in_port: port_number, dl_type: 0x0800, nw_dst: prefix),
      actions: SendOutPort.new(PORTS[:inspect]))
  end

  def install_postamble(priority)
    send_flow_mod_add(
      @dpid,
      priority: priority + 100,
      match: Match.new(in_port: PORTS[:inside]),
      actions: SendOutPort.new(PORTS[:outside]))
    send_flow_mod_add(
      @dpid,
      priority: priority,
      match: Match.new(in_port: PORTS[:outside]),
      actions: SendOutPort.new(PORTS[:inside]))
  end
end
