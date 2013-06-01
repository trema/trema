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


shared_examples_for "any stats-request" do
  it_should_behave_like "any Openflow message with default transaction ID"
  its( :flags ) { should == 0 }
end


describe StatsRequest do
  context "when .DescStatsRequest.new( VALID OPTIONS )" do
    subject { DescStatsRequest.new }
    it_should_behave_like "any stats-request"
  end


  context "when .FlowStatsRequest.new( MANDATORY OPTION MISSING )" do
    subject { FlowStatsRequest.new }
    it "should raise ArgumentError" do
      expect { subject }.to raise_error( ArgumentError )
    end
  end


  context "when .FlowStatsRequest.new( OPTIONAL OPTIONS MISSING )" do
    subject { FlowStatsRequest.new( :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ) ) }
    it_should_behave_like "any stats-request"
    its( :match ) { should be_an_instance_of( Match ) }
    its( :table_id ) { should == 0xff }
    its( :out_port ) { should == 0xffff }
  end


  context "when .FlowStatsRequest.new( VALID OPTIONS )" do
    subject do
      FlowStatsRequest.new(
        :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ),
        :table_id => 1,
        :out_port => 2
      )
    end
    it_should_behave_like "any stats-request"
    its( :match ) { should be_an_instance_of( Match ) }
    its( :table_id ) { should == 1 }
    its( :out_port ) { should == 2 }
  end


  context "when .AggregateStatsRequest.new( MANDATORY OPTION MISSING )" do
    subject { AggregateStatsRequest.new }
    it "should raise ArgumentError" do
      expect { subject }.to raise_error( ArgumentError )
    end
  end


  context "when .AggregateStatsRequest.new( OPTIONAL OPTIONS MISSING )" do
    subject { AggregateStatsRequest.new( :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ) ) }
    it_should_behave_like "any stats-request"
    its( :match ) { should be_an_instance_of( Match ) }
    its( :table_id ) { should == 0xff }
    its( :out_port ) { should == 0xffff }
  end


  context "when .AggregateStatsRequest.new( VALID OPTIONS )" do
    subject do
      AggregateStatsRequest.new(
        :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ),
        :table_id => 1,
        :out_port => 2
      )
    end
    it_should_behave_like "any stats-request"
    its( :match ) { should be_an_instance_of( Match ) }
    its( :table_id ) { should == 1 }
    its( :out_port ) { should == 2 }
  end


  context "when .TableStatsRequest.new( VALID OPTIONS )" do
    subject { TableStatsRequest.new }
    it_should_behave_like "any stats-request"
  end


  context "when .PortStasRequest.new( OPTIONAL OPTION MISSING )" do
    subject { PortStatsRequest.new }
    it_should_behave_like "any stats-request"
    its( :port_no ) { should == 0xffff }
  end


  context "when .PortStasRequest.new( VALID OPTIONS )" do
    subject { PortStatsRequest.new :port_no => 1 }
    it_should_behave_like "any stats-request"
    its( :port_no ) { should == 1 }
  end


  context "when .QueueStatsRequest.new( OPTIONAL OPTIONS MISSING )" do
    subject { QueueStatsRequest.new }
    it_should_behave_like "any stats-request"
    its( :port_no ) { should == 0xfffc }
    its( :queue_id ) { should == 0xffffffff }
  end


  context "when .QueueStatsRequest.new( VALID OPTIONS )" do
    subject { QueueStatsRequest.new :port_no => 1, :queue_id => 2 }
    it_should_behave_like "any stats-request"
    its( :port_no ) { should == 1 }
    its( :queue_id ) { should == 2 }
  end


  context "when .VendorStatsRequest.new( OPTIONAL OPTION MISSING )" do
    subject { VendorStatsRequest.new }
    it_should_behave_like "any stats-request"
    its( :vendor_id ) { should == 0x00004cff }
  end


  context "when .VendorStatsRequest.new( VALID OPTION )" do
    subject { VendorStatsRequest.new :vendor_id => 123 }
    it_should_behave_like "any stats-request"
    its( :vendor_id ) { should == 123 }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
