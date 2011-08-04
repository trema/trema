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


describe Trema::BarrierRequest do
  context "when an instance is created" do
    it "should automatically allocate a transaction ID" do
      barrier_request = Trema::BarrierRequest.new
      barrier_request.transaction_id.should be_a_kind_of( Integer )
      barrier_request.transaction_id.should >= 0
    end
    
    
    it "should have a valid transaction ID" do
      barrier_request = Trema::BarrierRequest.new( 1234 )
      barrier_request.transaction_id.should == 1234
    end
  end

  
  context "when #barrier_request is sent" do
    it "should receive #barrier_reply" do
      class BarrierController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( BarrierController ) {
        barrier_request = Trema::BarrierRequest.new( 1234 )
        controller( "BarrierController" ).send_message( 0xabc, barrier_request )
        controller( "BarrierController" ).should_receive( :barrier_reply )
      }
    end
  end
  

  context "when #barrier_request is sent with transaction_id" do
    it "should receive #barrier_reply with valid transaction_id" do
      class BarrierController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( BarrierController ) {
        barrier_request = Trema::BarrierRequest.new( 1234 )
        controller( "BarrierController" ).send_message( 0xabc, barrier_request )
        controller( "BarrierController" ).should_receive( :barrier_reply ) do | arg |
          arg.datapath_id.should == 0xabc
          arg.transaction_id.should == 1234
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
