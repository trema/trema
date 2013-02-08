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


describe SetConfig, "new( OPTIONAL OPTION MISSING )" do
  its( :flags ) { should == 0 }
  its( :miss_send_len ) { should == 128 }
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe SetConfig, ".new( VALID OPTIONS )" do
  subject { SetConfig.new( :flags => 1, :miss_send_len => 256, :transaction_id => transaction_id ) }
  let( :transaction_id ) { 123 }
  its( :flags ) { should  == 1 }
  its( :miss_send_len ) { should == 256 }
  it_should_behave_like "any OpenFlow message with transaction_id option"


  context "when #set_config is sent" do
    it "should not #set_config_reply" do
      class SetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( SetConfigController ) {
        set_config = SetConfig.new( :flags => 0, :miss_send_len => 128, :transaction_id => 123 )
        controller( "SetConfigController" ).should_not_receive( :set_config_reply )
        controller( "SetConfigController" ).send_message( 0xabc, set_config )
        sleep 2 # FIXME: wait to send_message
      }
    end
  end


  context "when #set_config is sent with flags, miss_send_len set to 0" do
    it "should #get_config_reply with flags, miss_send_len set to 0" do
      class SetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( SetConfigController ) {
        controller( "SetConfigController" ).should_receive( :get_config_reply ) do | dpid, arg |
          expect( arg.flags ).to eq( 0 )
          expect( arg.miss_send_len ).to eq( 0 )
        end
        set_config = SetConfig.new( :flags => 0, :miss_send_len => 0, :transaction_id => 123 )
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
