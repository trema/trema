#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


require "observer"
require "model"


module Trema
  module Router
    include PathResolver
    include Topology
    include Model


    def start_router options
      init_path_resolver
      init_topology name
      add_observer self
      @opts = options
      @model_ds = SwitchDS.new
      @fdb = FDB.new
    end


    def update message, kind
      case kind
      when :link_status
        process_link_status message
      when :port_status
        process_port_status message
      else
        raise "Invalid update message received"
      end
    end


    def packet_in datapath_id, message
      # abort processing if in_port is not known.
      return unless validate_in_port datapath_id, message.in_port
      return if message.macda.is_multicast?
      @fdb.learn message.macsa, message.in_port, datapath_id
      if dest = @fdb.lookup( message.macda )
        make_path datapath_id, message, dest
      else
        flood_packet datapath_id, message
      end
    end


    def switch_ready datapath_id
      send_message datapath_id, FeaturesRequest.new
    end


    def features_reply message
      send_message message.datapath_id, SetConfigRequest.new( :miss_send_len => 2**16 -1 )
    end


    def validate_in_port datapath_id, port
      @model_ds.validate_port datapath_id, port
    end


    def process_port_status message
      @model_ds.process_port_status message
    end


    def process_link_status message
      @model_ds.process_link_status message
      update_path message
    end


    def make_path in_datapath_id, message, dest
      out_datapath_id = dest[ 0 ]
      out_port = dest[ 1 ]
      if hops = path_resolve( in_datapath_id, message.in_port, out_datapath_id, out_port )
        modify_flow_entry hops, message
        output_packet_from_last_switch hops.last, message
      else
        discard_packet_in in_datapath_id, message
      end
    end


    def output_packet_from_last_switch last_hop, message 
      send_packet_out( last_hop.dpid, 
        :packet_in => message, 
        :actions => ActionOutput.new( :port => last_hop.out_port_no )
      )
    end


    def discard_packet_in datapath_id, message
      send_flow_mod_add( datapath_id, 
        :match => Match.from( message ),
        :hard_timeout => 1
      )
    end


    def modify_flow_entry hops, message
      nr_hops = hops.length
      hops.each do | hop |
        idle_timeout = @opts[ :idle_timeout ] + nr_hops 
        nr_hops -= 1
        send_flow_mod_add( hop.dpid,
          :match => Match.from( message ),
          :idle_timeout => idle_timeout,
          :actions => ActionOutput.new( :port => hop.out_port_no )
        )
      end
    end


    def flood_packet datapath_id, message
      @model_ds.each do | dpid, ports |
        actions = []
        ports.each do | port |
          next if port.include?( message.in_port ) and dpid == datapath_id
          actions << ActionOutput.new( :port => port.port_no ) if port.action_port?
        end
        send_packet_out( dpid, :packet_in => message, :actions => actions ) if actions.length
      end
    end
  end


  class RoutingSwitchOptions < Model::Options
    def parse!( args )
      super
      args.options do |opts|
      opts.on( "-i",
        "--idle-timeout TIMEOUT",
        "Idle timeout value for flow entry" ) do | t |
          @options[ :idle_timeout ] = t.to_i
        end
      end.parse!
      self
    end


    def default_options
      {
        :idle_timeout => 60
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
