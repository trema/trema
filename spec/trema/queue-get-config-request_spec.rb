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


describe QueueGetConfigRequest, ".new( OPTIONAL OPTION MISSING )" do
  its( :port ) { should == 1 }
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe QueueGetConfigRequest, ".new( VALID OPTIONS )" do
  subject { QueueGetConfigRequest.new( :transaction_id => transaction_id, :port => port ) }
  let( :transaction_id ) { 123 }
  let( :port ) { 2 }
  it_should_behave_like "any OpenFlow message with transaction_id option"
  it_should_behave_like "any OpenFlow message with port option"


  context "when #queue_get_config_request is sent" do
    it "should #queue_get_config_reply" do
      pending "#queue_get_config_reply is not implemented in #{Trema.vendor_openvswitch}"
      class QueueGetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( QueueGetConfigController ) {
        controller( "QueueGetConfigController" ).should_receive( :queue_get_config_reply )
        queue_get_config_request = QueueGetConfigRequest.new( :transaction_id => 123, :port => 1 )
        controller( "QueueGetConfigController" ).send_message( 0xabc, queue_get_config_request )
        sleep 2 # FIXME: wait to send_message
      }
    end
  end
end


describe QueueGetConfigRequest, ".new( INVALID OPTIONS )" do
  it "should raise a TypeError" do
    expect {
     QueueGetConfigRequest.new "INVALID OPTIONS"
    }.to raise_error( TypeError )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
