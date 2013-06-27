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


shared_examples_for "echo reply message" do
  class EchoReplyController < Controller; end

  it "should be logged to the switch's log", :sudo => true do
    network {
      vswitch( "echo" ) { datapath_id 0xabc }
    }.run( EchoReplyController ) {
      controller( "EchoReplyController" ).send_message( 0xabc, subject )
      sleep 2 # FIXME: wait to send_message
      expect( IO.read( File.join( Trema.log, "openflowd.echo.log" ) ) ).to include( "OFPT_ECHO_REPLY" )
    }
  end
end


module Trema
  describe EchoRequest, ".new" do
    it_should_behave_like "any Openflow message with default transaction ID"
    it_should_behave_like "echo reply message"
    its( :user_data ) { should be_nil }
  end


  describe EchoRequest, ".new(nil)" do
    subject { EchoRequest.new( nil ) }
    it_should_behave_like "any Openflow message with default transaction ID"
    it_should_behave_like "echo reply message"
    its( :user_data ) { should be_nil }
  end


  describe EchoRequest, ".new(transaction_id)" do
    subject { EchoRequest.new( transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"

    context "when sent to a switch" do
      let( :transaction_id ) { 123 }
      it_should_behave_like "echo reply message"
    end
  end


  describe EchoRequest, ".new(:transaction_id => value)" do
    subject { EchoRequest.new( :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"

    context "when sent to a switch" do
      let( :transaction_id ) { 123 }
      it_should_behave_like "echo reply message"
    end
  end


  describe EchoRequest, ".new(:xid => value)" do
    subject { EchoRequest.new( :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"

    context "when sent to a switch" do
      let( :xid ) { 123 }
      it_should_behave_like "echo reply message"
    end
  end


  describe EchoRequest, ".new(:user_data => value)" do
    subject { EchoRequest.new( :user_data => user_data ) }
    it_should_behave_like "any Openflow message with user_data"

    context "when sent to a switch" do
      let( :user_data ) { "USER DATA" }
      it_should_behave_like "echo reply message"
    end
  end


  describe EchoRequest, ".new(:transaction_id => value, :user_data => value)" do
    subject { EchoRequest.new( :transaction_id => transaction_id, :user_data => user_data ) }

    context 'transaction_id: 123, user_data: "USER DATA"', :nosudo => true do
      let( :transaction_id ) { 123 }
      let( :user_data ) { "USER DATA" }

      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
      its( :user_data ) { should == "USER DATA" }
    end

    context "when sent to a switch" do
      let( :transaction_id ) { 123 }
      let( :user_data ) { "USER DATA" }
      it_should_behave_like "echo reply message"
    end
  end


  describe EchoRequest, '.new("INVALID OPTION")' do
    it { expect { EchoRequest.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
