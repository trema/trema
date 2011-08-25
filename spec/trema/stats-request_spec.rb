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


shared_examples_for "#stats-request" do
  its( :transaction_id ) { should >= 0 }
  its( :flags ) { should == 0 }
end


describe Trema::StatsRequest do
  context "when #flow-stats is created" do
    subject { FlowStatsRequest.new( :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ) ) }
    it_should_behave_like "#stats-request"
    its( :match ) { should be_an_instance_of( Match ) }
    its( :table_id ) { should == 0xff }
    its( :out_port ) { should == 0xffff }
  end
  
  
  context "when #aggregate-stats is created" do
    subject { AggregateStatsRequest.new( :match => Match.new( :dl_type => 0x800, :nw_proto => 17 ) ) }
    it_should_behave_like "#stats-request"
    its( :match ) { should be_an_instance_of( Match ) }
    its( :table_id ) { should == 0xff }
    its( :out_port ) { should == 0xffff }
  end

  
  context "when #table-stats is created" do
    subject { TableStatsRequest.new() }
    it_should_behave_like "#stats-request"
  end
  
  
  context "when #port-stats is created" do
    subject { PortStatsRequest.new() } 
    it_should_behave_like "#stats-request"
    its( :port_no ) { should == 0xffff }
  end
  
  
  context "when #queue-stats is created" do
    subject { QueueStatsRequest.new() }
    it_should_behave_like "#stats-request"
    its( :port_no ) { should == 0xfffc }
    its( :queue_id ) { should == 0xffffffff }
  end
  
  
  context "when #vendor-stats is created" do
    subject { VendorStatsRequest.new() }
    it_should_behave_like "#stats-request"
    its( :vendor_id ) { should == 0x00004cff }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
