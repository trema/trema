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


describe GetConfigRequest, ".new" do
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe GetConfigRequest, ".new( :transaction_id => transaction_id )" do
  subject { GetConfigRequest.new :transaction_id => transaction_id }
  it_should_behave_like "any OpenFlow message"
end


describe GetConfigRequest, ".new( 1234 )" do
  subject { GetConfigRequest.new 1234 }
  it_should_behave_like "any incorrect signature constructor"
end


describe GetConfigRequest, ".new( :transaction_id => 1234 )" do
  context "when #get_config_request is sent" do
    it "should #get_config_reply" do
      class GetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( GetConfigController ) {
        get_config_request = GetConfigRequest.new( :transaction_id => 1234 )
        sleep 1 # FIXME
        controller( "GetConfigController" ).send_message( 0xabc, get_config_request )
        controller( "GetConfigController" ).should_receive( :get_config_reply )
        sleep 2 # FIXME: wait to send_message
      }
    end
    
    
    it "should #get_config_reply with valid attributes" do
      class GetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( GetConfigController ) {
        get_config_request = GetConfigRequest.new( :transaction_id => 1234 )
        controller( "GetConfigController" ).should_receive( :get_config_reply ) do | message |
          message.datapath_id.should == 0xabc
          message.transaction_id.should == 1234
          message.flags.should >= 0 and message.flags.should <= 3
          message.miss_send_len.should == 65535
        end
        controller( "GetConfigController" ).send_message( 0xabc, get_config_request )
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
