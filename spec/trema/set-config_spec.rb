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
    before( :all ) do
      @set_config = SetConfig.new
    end
    
    
    it "should have transaction_id" do
      @set_config.transaction_id.should >= 0
    end
    
    
    it "should have flags" do
      @set_config.flags.should == 0
    end
    
    
    it "should have miss_send_len" do
      @set_config.miss_send_len.should == 128
    end
  end
  
  
  context "when an instance is created with arguments" do
    before( :all ) do
      @set_config = SetConfig.new( 1234, 1, 256 )
    end
    
    
    it "should have transaction_id(1234)" do
      @set_config.transaction_id.should == 1234
    end
    
    
    it "should have flags(1)" do
      @set_config.flags.should == 1
    end
    
    
    it "should have miss_send_len(256)" do
      @set_config.miss_send_len.should == 256
    end
  end
  
  
  context "when creating with negative transaction ID(-1234)" do
    it "should raise an error" do
      lambda do 
        SetConfig.new( -1234, 0, 128 )
      end.should raise_error( "Transaction ID must be >= 0" )
    end
  end
  
  
  context "when #set_config is sent" do
    it "should not #set_config_reply" do
      class SetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( SetConfigController ) {
        set_config = SetConfig.new( 123, 0, 128 )
        sleep 1 # FIXME
        controller( "SetConfigController" ).send_message( 0xabc, set_config )
        controller( "SetConfigController" ).should_not_receive( :set_config_reply )
        sleep 2 # FIXME: wait to send_message
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
        sleep 1 # FIXME
        controller( "SetConfigController" ).send_message( 0xabc, set_config )
        controller( "SetConfigController" ).send_message( 0xabc, GetConfigRequest.new )
        controller( "SetConfigController" ).should_receive( :get_config_reply ) do | arg |
          arg.flags.should == 0
          arg.miss_send_len.should == 0
        end
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
