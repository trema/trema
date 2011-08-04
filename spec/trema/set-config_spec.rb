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


describe Trema::SetConfig do
  context "when an instance is created with no arguments" do
    it "should have valid default attributes" do
      set_config = SetConfig.new
      set_config.transaction_id.should be_a_kind_of Integer
      set_config.flags.should == 0
      set_config.miss_send_len.should == 128
    end
  end
  
  
  context "when an instance is created with arguments" do
    it "should have valid attributes" do
      set_config = SetConfig.new( 123, 1, 128 )
      set_config.transaction_id.should == 123
      set_config.flags.should == 1
      set_config.miss_send_len.should == 128
    end
  end
  
    
  context "when #set_config is sent" do
    it "should not receive #set_config_reply" do
      class SetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( SetConfigController ) {
        set_config = SetConfig.new( 123, 0, 128 )
        controller( "SetConfigController" ).send_message( 0xabc, set_config )
        controller( "SetConfigController" ).should_not_receive( :set_config_reply )
      }
    end
  end
  

  context "when #set_config is sent with flags and miss_send_len set to 0" do
    it "should have flags and miss_send_len set to 0 in #get_config_reply " do
      class SetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( SetConfigController ) {
        set_config = SetConfig.new( 123, 0, 0 )
        controller( "SetConfigController" ).send_message( 0xabc, set_config )
        controller( "SetConfigController" ).send_message( 0xabc, GetConfigRequest.new )
        controller( "SetConfigController" ).should_receive( :get_config_reply ) do | arg |
          arg.flags.should == 0
          arg.miss_send_len.should == 0
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
