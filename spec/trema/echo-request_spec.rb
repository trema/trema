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


describe EchoRequest, ".new( OPTIONAL OPTION MISSING )" do
  its( :user_data ) { should be_nil }
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe EchoRequest, ".new( INVALID OPTIONS ) - user_data numeric" do
  subject { EchoRequest.new 1234, 456 }
  it "should raise an ArgumentError" do
    expect { subject }.to raise_error( ArgumentError )
  end
end


describe EchoRequest, ".new( INVALID OPTIONS ) - arguments as an Array" do
  subject { EchoRequest.new [ 1234, "this is a test" ] }
  it "should raise a TypeError" do
    expect { subject }.to raise_error( TypeError )
  end
end


describe EchoRequest, ".new( VALID OPTIONS )" do
  subject { EchoRequest.new :transaction_id => transaction_id, :user_data => 'this is a test' }
  let( :transaction_id ) { 1234 }
  its( :user_data ) { should eq( "this is a test" ) }
  it_should_behave_like "any OpenFlow message with transaction_id option"


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
