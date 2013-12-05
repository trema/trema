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
  def start
    @dpid = nil
    @prefixes = ['10.0.0.0/8', '172.16.0.0/12', '192.168.0.0/16']
    @prefixes.map! { |each| Pio::IPv4Address.new(each) }
  end

  def switch_ready(dpid)
    unless @dpid.nil?
      info "#{dpid.to_hex}: ignored"
      return
    end
    @dpid = dpid
    info "#{@dpid.to_hex}: connected"
    @outside_port_no = 1
    @inside_port_no = 2
    @inspect_port_no = 3
    start_loading
  end

  def switch_disconnected(dpid)
    return if @dpid != dpid
    info "#{@dpid.to_hex}: disconnected"
    @dpid = nil
  end

  def start_loading
    @prefixes.each do |each|
      block_prefix_on_port(each, @inside_port_no, 5000)
      block_prefix_on_port(each, @outside_port_no, 4000)
    end
    install_postamble(1500)
    send_message(@dpid, BarrierRequest.new)
  end

  def block_prefix_on_port(pfx, port_no, prio)
    send_flow_mod_add(
      @dpid, priority: prio + 100,
             match: Match.new(in_port: port_no, dl_type: 0x0800, nw_src: pfx),
             actions: [SendOutPort.new(@inspect_port_no)])
    send_flow_mod_add(
      @dpid, priority: prio,
             match: Match.new(in_port: port_no, dl_type: 0x0800, nw_dst: pfx),
             actions: [SendOutPort.new(@inspect_port_no)])
  end

  def install_postamble(prio)
    send_flow_mod_add(
      @dpid, priority: prio + 100,
             match: Match.new(in_port: @inside_port_no),
             actions: [SendOutPort.new(@outside_port_no)])
    send_flow_mod_add(
      @dpid, priority: prio,
             match: Match.new(in_port: @outside_port_no),
             actions: [SendOutPort.new(@inside_port_no)])
  end

  def barrier_reply(dpid, message)
    return unless dpid == @dpid
    info "#{@dpid.to_hex}: loading finished"
  end
end
