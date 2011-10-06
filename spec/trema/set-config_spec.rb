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


describe SetConfig do
  its( :flags ) { should == 0 }
  its( :miss_send_len ) { should == 128 }
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe SetConfig, ".new( transaction_id, 1, 256 )" do
  subject { SetConfig.new transaction_id, 1, 256 }
  let( :transaction_id ) { 1234 }
  its( :flags ) { should  == 1 }
  its( :miss_send_len ) { should == 256 }
  it_should_behave_like "any OpenFlow message"
end  


describe SetConfig, ".new( 123, 0, 128 )" do
  context "when #set_config is sent" do
    it "should not #set_config_reply" do
      class SetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( SetConfigController ) {
        set_config = SetConfig.new( 123, 0, 128 )
        controller( "SetConfigController" ).should_not_receive( :set_config_reply )
        controller( "SetConfigController" ).send_message( 0xabc, set_config )
        sleep 2 # FIXME: wait to send_message
      }
    end
  end
end


describe SetConfig, ".new( 123, 0, 0 )" do
  context "when #set_config is sent with flags, miss_send_len set to 0" do
    it "should #get_config_reply with flags, miss_send_len set to 0" do
      class SetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( SetConfigController ) {
        controller( "SetConfigController" ).should_receive( :get_config_reply ) do | arg |
          arg.flags.should == 0
          arg.miss_send_len.should == 0
        end
        set_config = SetConfig.new( 123, 0, 0 )
        controller( "SetConfigController" ).send_message( 0xabc, set_config )
        sleep 2 # FIXME: wait to send_message
        controller( "SetConfigController" ).send_message( 0xabc, GetConfigRequest.new )
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
