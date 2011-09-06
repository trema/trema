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
  context "when #flow-stats-reply is created" do
    subject do
      actions = [ ActionOutput.new( 1 ) ]
      match = Match.new
      FlowStatsReply.new(
        :length => 96,
        :table_id => 0,
        :match => match,
        :duration_sec => 3,
        :duration_nsec => 106000000,
        :priority => 65535,
        :idle_timeout => 0,
        :hard_timeout => 0,
        :cookie => 866942928268820481,
        :packet_count => 2,
        :byte_count => 128,
        :actions => actions
      )
    end
    
    it { should respond_to( :to_s ) }
    its ( :length ) { should == 96 }
    its ( :table_id ) { should == 0 }
    its ( :match ) { should be_an_instance_of Match }
    its ( :duration_sec ) { should == 3 }
    its ( :duration_nsec ) { should == 106000000 }
    its ( :priority ) { should == 65535 } 
    its ( :idle_timeout ) { should == 0 }
    its ( :hard_timeout ) { should == 0 }
    its ( :cookie ) { should == 866942928268820481 }
    its ( :packet_count ) { should == 2 }
    its ( :byte_count ) { should == 128 }
    its ( :actions ) { should_not be_empty }
  end

  
  context "when aggregate-stats-reply is created" do
    subject do
      AggregateStatsReply.new(
        :packet_count => 2,
        :byte_count => 128,
        :flow_count =>  10 
      )
    end
    
    it { should respond_to( :to_s ) }
    its( :packet_count ) { should == 2 }
    its( :byte_count ) { should == 128 }
    its ( :flow_count ) { should == 10 }
  end
  
  
  context "when table-stats-reply is created" do
    subject do
      TableStatsReply.new(
        :table_id => 1,
        :name => "classifier",
        :wildcards => 4194303,
        :max_entries => 1048576,
        :active_count => 4,
        :lookup_count => 4,
        :matched_count => 1
      )
    end
    
    it { should respond_to( :to_s ) }
    its( :table_id ) { should == 1 }
    its( :name ) { should eq( "classifier" ) }
    its( :wildcards ) { should == 4194303 }
    its( :max_entries ) { should == 1048576 }
    its( :active_count ) { should == 4 }
    its( :lookup_count ) { should == 4 }
    its( :matched_count ) { should == 1 }
  end
  
  
  context "when port-stats-reply is created" do
    subject do
      PortStatsReply.new(
        :port_no => 1,
        :rx_packets => 7,
        :tx_packets => 10,
        :rx_bytes => 1454,
        :tx_bytes => 2314,
        :rx_dropped => 1,
        :tx_dropped => 1,
        :rx_errors => 1,
        :tx_errors => 1,
        :rx_frame_err => 1,
        :rx_over_err => 1,
        :rx_crc_err => 1,
        :collisions => 1
      )
    end
    
    it { should respond_to( :to_s ) }
    its( :port_no ) { should == 1 }
    its( :rx_packets ) { should == 7 }
    its( :tx_packets ) { should == 10 }
    its( :rx_bytes ) { should == 1454 }
    its( :tx_bytes ) { should == 2314 }
    its ( :rx_dropped ) { should == 1 }
    its( :tx_dropped ) { should == 1 }
    its( :rx_errors ) { should == 1 }
    its( :tx_errors ) { should == 1 }
    its( :rx_frame_err ) { should == 1 }
    its( :rx_over_err ) { should == 1 }
    its( :rx_crc_err ) { should == 1 }
    its( :collisions ) { should == 1 }
  end

  
  context "when queue-stats-reply is created" do
    subject do
      QueueStatsReply.new(
        :port_no => 1,
        :queue_id => 2,
        :tx_bytes => 1024,
        :tx_packets => 16,
        :tx_errors => 5 
      )
    end
    
    it { should respond_to( :to_s ) }
    its( :port_no ) { should == 1 }
    its( :queue_id ) { should == 2 }
    its( :tx_bytes ) { should == 1024 }
    its( :tx_packets ) { should == 16 }
    its( :tx_errors ) { should  == 5 }
  end
  
  
  context "when vendor-stats-reply is created" do
    subject { VendorStatsReply.new( :vendor_id => 123 ) }

    it { should respond_to( :to_s ) }
    its( :vendor_id ) { should == 123 }
  end
  
  
  context "when #stats_request(flow-stats) is sent" do
    it "should #stats_reply(flow-stats)" do
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
        sleep 2 # FIXME: wait to send_packets
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
    it "should #stats_reply(aggregate-stats) attributes" do
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
        sleep 2 # FIXME: wait to send_packets
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
    it "should #stats_reply(port-stats)" do
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
    it "should #stats_reply(table-stats)" do
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
          TableStatsRequest.new( :transaction_id => 123 ).to_packet.buffer )
        controller( "TableStatsController" ).should_receive( :stats_reply ) do | message |
          message.type.should == 3
          message.transaction_id.should == 123
          message.stats[0].should be_an_instance_of(Trema::TableStatsReply)
          message.stats[0].should respond_to :to_s
        end
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
