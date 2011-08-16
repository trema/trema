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


describe EchoRequest do
  context "when an instance is created" do  
    it "should automatically allocate a transaction ID" do
      echo_request = EchoRequest.new( )
      echo_request.transaction_id.should be_a_kind_of( Integer )
      echo_request.transaction_id.should >= 0
    end
    
    
    it "should have no user data" do
      EchoRequest.new.user_data.should be_nil
    end
    
    
    it "should have a valid transaction ID" do
      EchoRequest.new( 1234 ).transaction_id.should == 1234
    end
    
    it "should have a user data as specified" do
      EchoRequest.new( 1234, "this is a test" ).user_data.should match( /this is a test/ )
    end
    
    it "should raise an error if specified user data is not a string" do
      expect {
        EchoRequest.new( 1234, 456 )
      }.to raise_error( ArgumentError )
    end
  end  

  
  context "when #echo_request is sent" do
    it "should receive #echo_reply" do
      class EchoRequestController < Controller; end
      network {
        vswitch( "echo_request" ) { datapath_id 0xabc }
      }.run( EchoRequestController ) {
        echo_request = EchoRequest.new( 1234, "this is a test" )
        controller( "EchoRequestController" ).send_message( 0xabc, echo_request )
        log_file = Trema.log_directory + "/openflowd.echo_request.log"
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
