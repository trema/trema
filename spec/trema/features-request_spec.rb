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


shared_examples_for "features request message" do
  class FeaturesRequestController < Controller; end

  it "should be logged to the switch's log", :sudo => true do
    network {
      vswitch( "features-request" ) { datapath_id 0xabc }
    }.run( FeaturesRequestController ) {
      controller( "FeaturesRequestController" ).send_message( 0xabc, subject )
      sleep 2 # FIXME: wait to send_message
      expect( IO.read( File.join( Trema.log, "openflowd.features-request.log" ) ) ).to include( "OFPT_FEATURES_REQUEST" )
    }
  end
end


module Trema
  describe FeaturesRequest, ".new" do
    it_should_behave_like "any Openflow message with default transaction ID"
    it_should_behave_like "features request message"
  end


  describe FeaturesRequest, ".new(nil)" do
    subject { FeaturesRequest.new( nil ) }
    it_should_behave_like "any Openflow message with default transaction ID"
    it_should_behave_like "features request message"
  end


  describe FeaturesRequest, ".new(transaction_id)" do
    subject { FeaturesRequest.new( transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
    context "when sent to a switch" do
      let( :transaction_id ) { 123 }
      it_should_behave_like "features request message"
    end
  end


  describe FeaturesRequest, ".new(:transaction_id => value)" do
    subject { FeaturesRequest.new( :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
    context "when sent to a switch" do
      let( :transaction_id ) { 123 }
      it_should_behave_like "features request message"
    end
  end


  describe FeaturesRequest, ".new(:xid => value)" do
    subject { FeaturesRequest.new( :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"

    context "when sent to a switch" do
      let( :xid ) { 123 }
      it_should_behave_like "features request message"
    end
  end


  describe FeaturesRequest, '.new("INVALID OPTION")', :nosudo => true do
    it { expect { FeaturesRequest.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
