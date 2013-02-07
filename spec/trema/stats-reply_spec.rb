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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema"


describe StatsReply, ".new( VALID OPTIONS )" do
  context "when #desc-stats-reply is created" do
    subject do
      DescStatsReply.new(
        :mfr_desc => "NEC Corporation",
        :hw_desc => "no hardware description",
        :sw_desc => "version xx.xx",
        :serial_num => "1234",
        :dp_desc => "nec01"
      )
    end

    it { should respond_to( :to_s ) }
    its ( :mfr_desc ) { should eq( "NEC Corporation" ) }
    its ( :hw_desc ) { should eq( "no hardware description" ) }
    its ( :sw_desc ) { should eq( "version xx.xx" ) }
    its ( :serial_num ) { should eq( "1234" ) }
    its ( :dp_desc ) { should eq( "nec01" ) }
  end


  context "when #flow-stats-reply is created" do
    subject do
      actions = [ ActionOutput.new( :port => 1 ) ]
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


  context "when #stats_request(desc-stats) is sent" do
    it "should #stats_reply(desc-stats)" do
      class DescStatsController < Controller; end
      network {
        vswitch( "desc-stats" ) { datapath_id 0xabc }
      }.run( DescStatsController ) {
        controller( "DescStatsController" ).should_receive( :stats_reply ) do | datapath_id, message |
          expect( datapath_id ).to eq( 0xabc )
          expect( message.type ).to eq( 0 )
          expect( message.stats[ 0 ].mfr_desc ).to eq( "Nicira Networks, Inc." )
          expect( message.stats[ 0 ].hw_desc ).to eq( "Open vSwitch" )
          expect( message.stats[ 0 ] ).to respond_to :to_s
        end

        controller( "DescStatsController" ).send_message( 0xabc,
          DescStatsRequest.new( :transaction_id => 1234 ) )
        sleep 2 # FIXME: wait to send_message
      }
    end
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
          :actions => ActionOutput.new( :port => FlowStatsController::OFPP_FLOOD )
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        # send two packets
        send_packets "host1", "host2", :n_pkts => 2
        sleep 2 # FIXME: wait to send_packets

        controller( "FlowStatsController" ).should_receive( :stats_reply ) do | datapath_id, message |
          expect( datapath_id ).to eq( 0xabc )
          expect( message.type ).to eq( 1 )
          expect( message.stats[ 0 ].packet_count ).to eq( 2 )
          expect( message.stats[ 0 ] ).to respond_to :to_s
        end
        match = Match.new( :dl_type =>0x800, :nw_proto => 17 )
        controller( "FlowStatsController" ).send_message( 0xabc,
          FlowStatsRequest.new( :match => match ) )
        sleep 2 # FIXME: wait to send_message
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
          :actions => ActionOutput.new( :port => AggregateStatsController::OFPP_FLOOD )
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        # send ten packets
        send_packets "host1", "host2", :n_pkts => 10
        sleep 2 # FIXME: wait to send_packets

        controller( "AggregateStatsController" ).should_receive( :stats_reply ) do | datapath_id, message |
          expect( datapath_id ).to eq( 0xabc )
          expect( message.type ).to eq( 2 )
          expect( message.stats[ 0 ].packet_count ).to eq( 10 )
          expect( message.stats[ 0 ].flow_count ).to eq( 1 )
          expect( message.stats[ 0 ] ).to respond_to :to_s
        end
        match = Match.new( :dl_type =>0x800, :nw_proto => 17 )
        controller( "AggregateStatsController" ).send_message( 0xabc,
          AggregateStatsRequest.new( :match => match, :table_id => 0xff ) )
        sleep 2 # FIXME: wait to send_message
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
          :actions => ActionOutput.new( :port => PortStatsController::OFPP_FLOOD )
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        send_packets "host1", "host2"
        sleep 2 # FIXME: wait to send_packets

        controller( "PortStatsController" ).should_receive( :stats_reply ) do | datapath_id, message |
          expect( datapath_id ).to eq( 0xabc )
          expect( message.type ).to eq( 4 )
          expect( message.stats[ 0 ] ).to be_an_instance_of( Trema::PortStatsReply )
          expect( message.stats[ 0 ] ).to respond_to :to_s
        end
        controller( "PortStatsController" ).send_message( 0xabc,
          PortStatsRequest.new( :port_no => 1 ) )
        sleep 2 # FIXME: wait to send_message
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
        controller( "TableStatsController" ).send_flow_mod_add(
          0xabc,
          :actions => ActionOutput.new( :port => TableStatsController::OFPP_FLOOD )
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        send_packets "host1", "host2"
        sleep 2 # FIXME: wait to send_packets

        controller( "TableStatsController" ).should_receive( :stats_reply ) do | datapath_id, message |
          expect( datapath_id ).to eq( 0xabc )
          expect( message.type ).to eq( 3 )
          expect( message.transaction_id ).to eq( 123 )
          expect( message.stats[ 0 ] ).to be_an_instance_of(Trema::TableStatsReply)
          expect( message.stats[ 0 ] ).to respond_to :to_s
        end
        controller( "TableStatsController" ).send_message( 0xabc,
          TableStatsRequest.new( :transaction_id => 123 ) )
        sleep 2 # FIXME: wait to send_message
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
