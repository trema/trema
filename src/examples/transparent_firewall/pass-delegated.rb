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

# A sample transparent firewall, see README.md for more information.
class PassDelegated < Controller
  PORTS = {
    outside: 1,
    inside: 2,
    inspect: 3
  }

  PRIORITIES = {
    bypass: 65_000,
    prefix: 64_000,
    inspect: 1000,
    non_ipv4: 900
  }

  PREFIX_FILES = %w[afrinic apnic arin lacnic ripencc].map do |each|
    "aggregated-delegated-#{each}.txt"
  end

  add_periodic_timer_event :request_flow_stats, 30

  def start
    @prefixes = PREFIX_FILES.reduce([]) do |result, each|
      data = IO.readlines(File.join(File.dirname(__FILE__), each))
      info "#{each}: #{data.size} prefix(es)"
      result + data
    end
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
      when PRIORITIES[:prefix]
        @stats.push each if each.byte_count > 0
      when PRIORITIES[:inspect]
        @denied_bytes = each.byte_count
      end
    end
  end

  private

  def start_loading
    install_preamble_and_bypass
    install_prefixes
    install_postamble
    @state = :loading
    @loading_started = Time.now
    send_message(@dpid, BarrierRequest.new)
  end

  # All flows in place, safe to remove bypass.
  def finish_loading
    send_flow_mod_delete(
      @dpid,
      strict: true,
      priority: PRIORITIES[:bypass],
      match: Match.new(in_port: PORTS[:outside]))
    info '%s: bypass OFF', @dpid.to_hex
    info('%s: loading finished in %.2f second(s)',
         @dpid.to_hex, Time.now - @loading_started)
    @denied_bytes = 0
    @state = :running
  end

  def install_preamble_and_bypass
    send_flow_mod_add(
      @dpid,
      priority: PRIORITIES[:bypass],
      match: Match.new(in_port: PORTS[:inside]),
      actions: SendOutPort.new(PORTS[:outside]))
    send_flow_mod_add(
      @dpid, priority: PRIORITIES[:bypass],
             match: Match.new(in_port: PORTS[:outside]),
             actions: SendOutPort.new(PORTS[:inside]))
    info "#{@dpid.to_hex}: bypass ON"
  end

  def install_prefixes
    info "#{@dpid.to_hex}: loading started"
    @prefixes.each do |each|
      send_flow_mod_add(
        @dpid, priority: PRIORITIES[:prefix],
               match: Match.new(in_port: PORTS[:outside],
                                dl_type: 0x0800,
                                nw_src: Pio::IPv4Address.new(each)),
               actions: SendOutPort.new(PORTS[:inside]))
    end
  end

  # Deny any other IPv4 and permit non-IPv4 traffic.
  def install_postamble
    send_flow_mod_add(
      @dpid,
      priority: PRIORITIES[:inspect],
      match: Match.new(in_port: PORTS[:outside], dl_type: 0x0800),
      actions: SendOutPort.new(PORTS[:inspect]))
    send_flow_mod_add(
      @dpid,
      priority: PRIORITIES[:non_ipv4],
      match: Match.new(in_port: PORTS[:outside]),
      actions: SendOutPort.new(PORTS[:inside]))
  end

  def dump_top_flows
    return unless @stats.size > 0
    info 'top-10 permitted source prefixes by bytes'
    @stats.first(10).each do |each|
      info('%15s/%-2u %u',
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
      @dpid,
      FlowStatsRequest.new(match: Match.new(in_port: PORTS[:outside])))
    send_message @dpid, BarrierRequest.new
  end
end
