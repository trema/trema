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


describe QueueGetConfigRequest do
  it_should_behave_like "any Openflow message with default transaction ID"
end  


describe QueueGetConfigRequest, ".new( transaction_id, 2 )" do
  subject { QueueGetConfigRequest.new( transaction_id, 2 ) }
  let( :transaction_id ) { 1234 }
  its( :port) { should == 2 }
  it_should_behave_like "any OpenFlow message"
end


describe QueueGetConfigRequest, ".new( 123, 1 )" do
  context "when #queue_get_config_request is sent" do
    it "should #queue_get_config_reply" do
      pending "#queue_get_config_reply is not implemented in #{Trema::Vendor::openvswitch}"
      class QueueGetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( QueueGetConfigController ) {
        controller( "QueueGetConfigController" ).should_receive( :queue_get_config_reply )
        queue_get_config_request = QueueGetConfigRequest.new( 123, 1 )
        controller( "QueueGetConfigController" ).send_message( 0xabc, queue_get_config_request )
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
