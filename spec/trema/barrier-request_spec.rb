#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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


describe BarrierRequest, ".new( OPTIONAL OPTION MISSING )" do
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe BarrierRequest, ".new( VALID OPTION )" do
  subject { BarrierRequest.new :transaction_id => transaction_id }
  it_should_behave_like "any OpenFlow message with transaction_id option"


  context "when #barrier_request" do
    it "should #barrier_reply" do
      class BarrierController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( BarrierController ) {
        controller( "BarrierController" ).should_receive( :barrier_reply )
        controller( "BarrierController" ).send_message( 0xabc, BarrierRequest.new )
        sleep 2 # FIXME: wait to send_message
      }
    end
  end
end


describe BarrierRequest, ".new( OPTIONAL OPTION ) - transaction_id" do
  context "when #barrier_request" do
    it "should #barrier_reply with transaction_id == value" do
      class BarrierController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( BarrierController ) {
        controller( "BarrierController" ).should_receive( :barrier_reply ) do | message |
          message.datapath_id.should == 0xabc
          message.transaction_id.should == 1234
        end
        barrier_request = BarrierRequest.new( :transaction_id => 1234 )
        controller( "BarrierController" ).send_message( 0xabc, barrier_request )
        sleep 2 # FIXME: wait to send_message
      }
    end
  end
end


describe BarrierRequest, ".new( INVALID_OPTION )" do
  it "should raise TypeError" do
    expect {
      BarrierRequest.new "INVALID_OPTION"
    }.to raise_error( TypeError )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
