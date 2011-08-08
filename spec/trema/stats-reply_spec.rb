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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema"


describe StatsReply do
  context "when #stats_request(flow-stats) is sent" do
    it "should receive #stats_reply with valid FlowStatsReply attributes" do
      class FlowStatsController < Controller; end
      network { 
        vswitch( "flow-stats" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "flow-stats"
        link "host2", "flow-stats"
      }.run( FlowStatsController ) {
        controller( "FlowStatsController" ).send_flow_mod_add( 
          0xabc, 
          # match the UDP packet
          :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ), 
          # flood the packet
          :actions => ActionOutput.new( FlowStatsController::OFPP_FLOOD ) )        
        send_packets "host1", "host2", :n_pkts => 2
        sleep 1 # FIXME: wait to send_packets
        match = Match.new( :dl_type =>0x800, :nw_proto => 17 )
        controller( "FlowStatsController" ).send_message( 0xabc, 
          FlowStatsRequest.new( :match => match ).to_packet.buffer )
        controller( "FlowStatsController" ).should_receive( :stats_reply ) do | message |
          message.type.should == 1
          message.stats[0].packet_count.should == 2
          message.stats[0].should respond_to :to_s
        end
      }
    end
  end
  
  
  context "when #stats_request(aggregate_stats) is sent" do
    it "should receive #stats_reply with valid AggregateStatsReply attributes" do
      class AggregateStatsController < Controller; end
      network { 
        vswitch( "aggregate-stats" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "aggregate-stats"
        link "host2", "aggregate-stats"
      }.run( AggregateStatsController ) {
        controller( "AggregateStatsController" ).send_flow_mod_add( 
          0xabc, 
          # match the UDP packet
          :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ), 
          # flood the packet
          :actions => ActionOutput.new( AggregateStatsController::OFPP_FLOOD ) )        
        # send two packets 
        send_packets "host1", "host2", :n_pkts => 10
        sleep 1 # FIXME: wait to send_packets
        match = Match.new( :dl_type =>0x800, :nw_proto => 17 )
        controller( "AggregateStatsController" ).send_message( 0xabc, 
          AggregateStatsRequest.new( :match => match, :table_id => 0xff ).to_packet.buffer )
        controller( "AggregateStatsController" ).should_receive( :stats_reply ) do | message |
          message.type.should == 2
          message.stats[0].packet_count.should == 10
          message.stats[0].flow_count.should == 1
          message.stats[0].should respond_to :to_s
        end
      }
    end
  end
  
  
  context "when #stats_request(port-stats) is sent" do
    it "should receive #stats_reply with valid PortStatsReply attributes" do
      class PortStatsController < Controller; end
      network { 
        vswitch( "port-stats" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "port-stats"
        link "host2", "port-stats"
      }.run( PortStatsController) {
        controller( "PortStatsController" ).send_flow_mod_add( 
          0xabc, 
          :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ), 
          :actions => ActionOutput.new( PortStatsController::OFPP_FLOOD ) )
        send_packets "host1", "host2"
        
        controller( "PortStatsController" ).send_message( 0xabc, 
          PortStatsRequest.new( :port_no => 1 ).to_packet.buffer )
        controller( "PortStatsController" ).should_receive( :stats_reply ) do | message |
          message.type.should == 4
          message.stats[0].should be_an_instance_of(Trema::PortStatsReply)
          message.stats[0].should respond_to :to_s
        end
      }
    end
  end
  
  
  context "when #stats_request(table-stats) is sent" do
    it "should receive #stats_reply with valid TableStatsReply attributes" do
      class TableStatsController < Controller; end
      network { 
        vswitch( "table-stats" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "table-stats"
        link "host2", "table-stats"
      }.run( TableStatsController) {
        controller( "TableStatsController" ).send_flow_mod_add( 0xabc, 
          :actions => ActionOutput.new( TableStatsController::OFPP_FLOOD ) )
        send_packets "host1", "host2"
        
        controller( "TableStatsController" ).send_message( 0xabc, 
          TableStatsRequest.new(:transaction_id => 123).to_packet.buffer )
        controller( "TableStatsController" ).should_receive( :stats_reply ) do | message |
          message.type.should == 3
          message.transaction_id.should == 123
          message.stats[0].should be_an_instance_of(Trema::TableStatsReply)
          message.stats[0].should respond_to :to_s
        end
      }
    end
  end
  
  
  context "when an instance of QueueStatsReply is created" do
    it "should respond to #to_s in super StatsReply" do
      queue_stats_options = { 
        :port_no => 1,
        :queue_id => 123
      }	
      queue_stats_reply = QueueStatsReply.new( queue_stats_options )
      stats_reply = StatsReply.new( :stats => [ queue_stats_reply ] )
      stats_reply.stats[0].should respond_to :to_s
    end
  end
  
  
  context "when an instance of VendorStatsReply is created" do
    it "should respond to #to_s in super StatsReply" do
      vendor_stats_reply = VendorStatsReply.new( :vendor_id => 123 )
      stats_reply = StatsReply.new( :stats => [ vendor_stats_reply ] )
      stats_reply.stats.should have_exactly(1).items
      stats_reply.stats[0].should respond_to :to_s
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
