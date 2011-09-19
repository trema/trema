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


describe GetConfigRequest do
  context "when an instance is created" do  
    it "should automatically allocate a transaction ID" do
      get_config_request = GetConfigRequest.new
      get_config_request.transaction_id.should be_a_kind_of( Integer )
      get_config_request.transaction_id.should >= 0
    end
    
    
    it "should have a valid transaction ID" do
      GetConfigRequest.new( 1234 ).transaction_id.should == 1234
    end
  end  

  
  context "when creating with negative transaction ID(-1234)" do
    it "should raise an error" do
      lambda do 
        GetConfigRequest.new( -1234 )
      end.should raise_error( "Transaction ID must be >= 0" )
    end
  end
  
  
  context "when #get_config_request is sent" do
    it "should #get_config_reply" do
      class GetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( GetConfigController ) {
        get_config_request = GetConfigRequest.new( 1234 )
        controller( "GetConfigController" ).send_message( 0xabc, get_config_request )
        sleep 2 # FIXME: wait to send_message
        controller( "GetConfigController" ).should_receive( :get_config_reply )
      }
    end
    
    
    it "should #get_config_reply with valid attributes" do
      class GetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( GetConfigController ) {
        get_config_request = GetConfigRequest.new( 1234 )
        controller( "GetConfigController" ).send_message( 0xabc, get_config_request )
        controller( "GetConfigController" ).should_receive( :get_config_reply ) do | message |
          message.datapath_id.should == 0xabc
          message.transaction_id.should == 1234
          message.flags.should >= 0 and message.flags.should <= 3
          message.miss_send_len.should == 65535
        end
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
