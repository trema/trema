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


describe BarrierRequest do
  context "when an instance is created with no arguments" do
    its( :transaction_id ) { should be_a_kind_of( Integer ) }
    its( :transaction_id ) { should >= 0 }
  end
  
  
  context "when an instance is created with transaction_id" do
    subject { BarrierRequest.new( 1234 ) }
    its( :transaction_id ) { should == 1234 }
  end

  
  context "when #barrier_request" do
    it "should #barrier_reply" do
      class BarrierController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( BarrierController ) {
        barrier_request = BarrierRequest.new( 1234 )
        sleep 1 # FIXME
        controller( "BarrierController" ).send_message( 0xabc, barrier_request )
        controller( "BarrierController" ).should_receive( :barrier_reply )
        sleep 2 # FIXME: wait to send_message
      }
    end
  end
  
  
  context "when #barrier_request with transaction_id" do
    it "should #barrier_reply with valid transaction_id" do
      class BarrierController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( BarrierController ) {
        barrier_request = BarrierRequest.new( 1234 )
        sleep 1 # FIXME
        controller( "BarrierController" ).send_message( 0xabc, barrier_request )
        controller( "BarrierController" ).should_receive( :barrier_reply ) do | message |
          message.datapath_id.should == 0xabc
          message.transaction_id.should == 1234
        end
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
