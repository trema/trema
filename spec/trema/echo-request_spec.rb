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


module Trema
  describe EchoRequest, ".new", :nosudo => true do
    its( :transaction_id ) { should be_unsigned_32bit }
    its( :xid ) { should be_unsigned_32bit }
    its( :user_data ) { should be_nil }
  end


  describe EchoRequest, ".new(nil)", :nosudo => true do
    subject { EchoRequest.new( nil ) }
    its( :transaction_id ) { should be_unsigned_32bit }
    its( :xid ) { should be_unsigned_32bit }
    its( :user_data ) { should be_nil }
  end


  describe EchoRequest, ".new(transaction_id)", :nosudo => true do
    subject { EchoRequest.new( transaction_id ) }

    context "transaction_id: -123" do
      let( :transaction_id ) { -123 }
      it { expect { subject }.to raise_error( ArgumentError, "Transaction ID must be an unsigned 32-bit integer" ) }
    end

    context "transaction_id: 0" do
      let( :transaction_id ) { 0 }
      its( :transaction_id ) { should == 0 }
      its( :xid ) { should == 0 }
    end

    context "transaction_id: 123" do
      let( :transaction_id ) { 123 }
      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
    end

    context "transaction_id: UINT32_MAX" do
      let( :transaction_id ) { 2 ** 32 - 1 }
      its( :transaction_id ) { should == 2 ** 32 - 1 }
      its( :xid ) { should == 2 ** 32 - 1 }
    end

    context "transaction_id: UINT32_MAX + 1" do
      let( :transaction_id ) { 2 ** 32 }
      it { expect { subject }.to raise_error( ArgumentError, "Transaction ID must be an unsigned 32-bit integer" ) }
    end
  end


  describe EchoRequest, ".new(:transaction_id => value)", :nosudo => true do
    subject { EchoRequest.new( :transaction_id => transaction_id ) }

    context "transaction_id: -123" do
      let( :transaction_id ) { -123 }
      it { expect { subject }.to raise_error( ArgumentError, "Transaction ID must be an unsigned 32-bit integer" ) }
    end

    context "transaction_id: 0" do
      let( :transaction_id ) { 0 }
      its( :transaction_id ) { should == 0 }
      its( :xid ) { should == 0 }
    end

    context "transaction_id: 123" do
      let( :transaction_id ) { 123 }
      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
    end

    context "transaction_id: UINT32_MAX" do
      let( :transaction_id ) { 2 ** 32 - 1 }
      its( :transaction_id ) { should == 2 ** 32 - 1 }
      its( :xid ) { should == 2 ** 32 - 1 }
    end

    context "transaction_id: UINT32_MAX + 1" do
      let( :transaction_id ) { 2 ** 32 }
      it { expect { subject }.to raise_error( ArgumentError, "Transaction ID must be an unsigned 32-bit integer" ) }
    end
  end


  describe EchoRequest, ".new(:xid => value)", :nosudo => true do
    subject { EchoRequest.new( :xid => xid ) }

    context "xid: -123" do
      let( :xid ) { -123 }
      it { expect { subject }.to raise_error( ArgumentError, "Transaction ID must be an unsigned 32-bit integer" ) }
    end

    context "xid: 0" do
      let( :xid ) { 0 }
      its( :xid ) { should == 0 }
      its( :transaction_id ) { should == 0 }
    end

    context "xid: 123" do
      let( :xid ) { 123 }
      its( :xid ) { should == 123 }
      its( :transaction_id ) { should == 123 }
    end

    context "xid: UINT32_MAX" do
      let( :xid ) { 2 ** 32 - 1 }
      its( :xid ) { should == 2 ** 32 - 1 }
      its( :transaction_id ) { should == 2 ** 32 - 1 }
    end

    context "xid: UINT32_MAX + 1" do
      let( :xid ) { 2 ** 32 }
      it { expect { subject }.to raise_error( ArgumentError, "Transaction ID must be an unsigned 32-bit integer" ) }
    end
  end


  describe EchoRequest, ".new(:user_data => value)", :nosudo => true do
    subject { EchoRequest.new( :user_data => user_data ) }

    context "user_data: nil" do
      let( :user_data ) { nil }
      its( :user_data ) { should be_nil }
      its( :transaction_id ) { should be_unsigned_32bit }
      its( :xid ) { should be_unsigned_32bit }
    end


    context 'user_data: "USER DATA"' do
      let( :user_data ) { "USER DATA" }
      its( :user_data ) { should == "USER DATA" }
      its( :transaction_id ) { should be_unsigned_32bit }
      its( :xid ) { should be_unsigned_32bit }
    end


    context "user_data: :INVALID_DATA" do
      let( :user_data ) { :INVALID_DATA }
      it { expect { subject }.to raise_error( ArgumentError, "User data must be a string" ) }
    end
  end


  describe EchoRequest, ".new(:transaction_id => value, :user_data => value)", :nosudo => true do
    subject { EchoRequest.new( :transaction_id => transaction_id, :user_data => user_data ) }

    context 'transaction_id: 123, user_data: "USER DATA"' do
      let( :transaction_id ) { 123 }
      let( :user_data ) { "USER DATA" }
      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
      its( :user_data ) { should == "USER DATA" }
    end
  end


  describe EchoRequest, '.new("INVALID OPTION")', :nosudo => true do
    it { expect { EchoRequest.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


describe EchoRequest do
  context "when #echo_request is sent" do
    it "should #echo_reply" do
      class EchoRequestController < Controller; end
      network {
        vswitch( "echo_request" ) { datapath_id 0xabc }
      }.run( EchoRequestController ) {
        echo_request = EchoRequest.new( :transaction_id => 1234, :user_data => 'this is a test' )
        controller( "EchoRequestController" ).send_message( 0xabc, echo_request )
        log_file = Trema.log + "/openflowd.echo_request.log"
        IO.read( log_file ).should include( "OFPT_ECHO_REPLY" )
        sleep 1
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
