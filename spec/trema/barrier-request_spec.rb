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


shared_examples_for "barrier request message" do
  class BarrierRequestController < Controller; end

  it "should be logged to the switch's log", :sudo => true do
    network {
      vswitch( "barrier-request" ) { datapath_id 0xabc }
    }.run( BarrierRequestController ) {
      controller( "BarrierRequestController" ).send_message( 0xabc, subject )
      sleep 2 # FIXME: wait to send_message
      expect( IO.read( File.join( Trema.log, "openflowd.barrier-request.log" ) ) ).to include( "OFPT_BARRIER_REQUEST" )
    }
  end
end


module Trema
  describe BarrierRequest, ".new" do
    it_should_behave_like "any Openflow message with default transaction ID"
    it_should_behave_like "barrier request message"
  end


  describe BarrierRequest, ".new(nil)" do
    subject { BarrierRequest.new( nil ) }
    it_should_behave_like "any Openflow message with default transaction ID"
    it_should_behave_like "barrier request message"
  end


  describe BarrierRequest, ".new(transaction_id)" do
    subject { BarrierRequest.new( transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
    context "when sent to a switch" do
      let( :transaction_id ) { 123 }
      it_should_behave_like "barrier request message"
    end
  end


  describe BarrierRequest, ".new(:transaction_id => value)" do
    subject { BarrierRequest.new( :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
    context "when sent to a switch" do
      let( :transaction_id ) { 123 }
      it_should_behave_like "barrier request message"
    end
  end


  describe BarrierRequest, ".new(:xid => value)" do
    subject { BarrierRequest.new( :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"
    context "when sent to a switch" do
      let( :xid ) { 123 }
      it_should_behave_like "barrier request message"
    end
  end


  describe BarrierRequest, '.new("INVALID OPTION")', :nosudo => true do
    it { expect { BarrierRequest.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
