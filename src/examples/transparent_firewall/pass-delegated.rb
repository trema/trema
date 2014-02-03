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

class PassDelegated < Controller
  add_periodic_timer_event :request_flow_stats, 30

  def start
    @prefixes = []
    %w[afrinic apnic arin lacnic ripencc].each do |each|
      @prefixes += prefixes_from_file("aggregated-delegated-#{each}.txt")
    end
    @dpid = nil
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

  def barrier_reply(dpid, message)
    return unless dpid == @dpid
    case @state
    when :loading then finish_loading
    when :running then dump_flow_stats
    else fail
    end
  end

  def stats_reply(dpid, message)
    return unless dpid == @dpid && message.type == StatsReply::OFPST_FLOW
    message.stats.each do |each|
      case each.priority
      when 64_000
        @stats.push each if each.byte_count > 0
      when 1000
        @denied_bytes = each.byte_count
      end
    end
  end

  private

  def prefixes_from_file(filename)
    ret = IO.readlines(File.join(File.dirname(__FILE__), filename))
    info "#{filename}: #{ret.size} prefix(es)"
    ret
  end

  def start_loading
    install_preamble_and_bypass
    info "#{@dpid.to_hex}: bypass ON, loading started"
    install_prefixes
    install_postamble
    @state = :loading
    @loading_started = Time.now
    send_message(@dpid, BarrierRequest.new)
  end

  # All flows in place, safe to remove bypass.
  def finish_loading
    send_flow_mod_delete(
                         @dpid, strict: true,
                         priority: 65_000,
                         match: Match.new(in_port: @outside_port_no))
    info sprintf('%s: bypass OFF, loading finished in %.2f second(s)',
                 @dpid.to_hex,
                 Time.now - @loading_started)
    @denied_bytes = 0
    @state = :running
  end

  def install_preamble_and_bypass
    send_flow_mod_add(
                      @dpid, priority: 65_000,
                      match: Match.new(in_port: @inside_port_no),
                      actions: [SendOutPort.new(@outside_port_no)])
    send_flow_mod_add(
                      @dpid, priority: 65_000,
                      match: Match.new(in_port: @outside_port_no),
                      actions: [SendOutPort.new(@inside_port_no)])
  end

  def install_prefixes
    @prefixes.each do |each|
      send_flow_mod_add(
                        @dpid, priority: 64_000,
                        match: Match.new(in_port: @outside_port_no,
                                         dl_type: 0x0800,
                                         nw_src: Pio::IPv4Address.new(each)),
                        actions: [SendOutPort.new(@inside_port_no)])
    end
  end

  # Deny any other IPv4 and permit non-IPv4 traffic.
  def install_postamble
    send_flow_mod_add(
                      @dpid, priority: 1_000,
                      match: Match.new(in_port: @outside_port_no, dl_type: 0x0800),
                      actions: [SendOutPort.new(@inspect_port_no)])
    send_flow_mod_add(
                      @dpid, priority: 900,
                      match: Match.new(in_port: @outside_port_no),
                      actions: [SendOutPort.new(@inside_port_no)])
  end

  def dump_top_flows
    return unless @stats.size > 0
    info 'top-10 permitted source prefixes by bytes'
    @stats.first(10).each do |each|
      info sprintf('%15s/%-2u %u',
                   each.match.nw_src.to_s,
                   each.match.nw_src.prefixlen,
                   each.byte_count)
    end
  end

  def dump_flow_stats
    @stats.sort! do |a, b|
      1 if a.byte_count < b.byte_count
      -1 if a.byte_count > b.byte_count
      0
    end
    dump_top_flows
    info "total denied bytes: #{@denied_bytes}" if @denied_bytes > 0
    @stats = []
  end

  # Interested in flows with and without IPv4 prefixes.
  def request_flow_stats
    return if @dpid.nil? || @state != :running
    @stats = []
    send_message(
                 @dpid, FlowStatsRequest.new(match: Match.new(in_port: @outside_port_no)))
    send_message(@dpid, BarrierRequest.new)
  end
end
