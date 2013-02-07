#
# Sample of the Match.compre
#
# Copyright (C) 2008-2011 NEC Corporation
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


class MatchCompare < Controller
  def start
    @rules = []
    add_rules
  end


  def packet_in datapath_id, message
    match = ExactMatch.from( message )
    action, log = lookup_rules( datapath_id, match )
    info "action=#{ action }, datapath_id=#{ datapath_id.to_hex }, message={#{ match.to_s }}" if log
    if action == :allow
      actions = ActionOutput.new( OFPP_FLOOD )
      send_flow_mod_add( datapath_id, :match => match, :idle_timeout => 60, :actions => actions )
      send_packet_out( datapath_id, :packet_in => message, :actions => actions )
    else
      send_flow_mod_add( datapath_id, :match => match, :idle_timeout => 60 )
    end
  end


  private


  def add_rules
    dl_type_arp = 0x0806
    dl_type_ipv4 = 0x0800
    network = "192.168.0.0/16"

    allow :dl_type => dl_type_arp
    allow :dl_type => dl_type_ipv4, :nw_src => network, :log => true
    allow :dl_type => dl_type_ipv4, :nw_dst => network, :log => true
    block :log => true
  end


  def allow hash = {}
    add_rule :allow, hash
  end


  def block hash = {}
    add_rule :block, hash
  end


  def add_rule action, hash
    datapath_id = hash.key?( :datapath_id ) && hash.delete( :datapath_id ) || nil
    log = hash.key?( :log ) && hash.delete( :log ) || false
    rule = Struct.new( :action, :datapath_id, :match, :log )
    @rules << rule.new( action, datapath_id, Match.new( hash ), log )
  end


  def lookup_rules datapath_id, match
    action = :block # default action
    log = false
    @rules.each do | each |
      if !each.datapath_id.nil? && datapath_id != each.datapath_id
        next
      end
      if each.match.compare( match )
        action = each.action
        log = each.log
        break
      end
    end
    return action, log
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
