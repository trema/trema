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


describe Trema::QueueGetConfigRequest do
  context "when an instance is created with no arguments" do
    it "should have a valid port  attributes" do
      queue_get_config_request = QueueGetConfigRequest.new
      queue_get_config_request.port.should == 1
    end
  end
  
  
  context "when an instance is created with arguments" do
    it "should have valid attributes" do
      queue_get_config_request = QueueGetConfigRequest.new( 123, 2 )
      queue_get_config_request.transaction_id.should == 123
      queue_get_config_request.port.should == 2
    end
  end
  
  
  context "when #queue_get_config_request is sent" do
    it "should receive #queue_get_config_reply" do
      pending "#queue_get_config_reply is not implemented in openvswitch-1.1.2"
      class QueueGetConfigController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( QueueGetConfigController ) {
        queue_get_config_request = QueueGetConfigRequest.new( 123, 1 )
        controller( "QueueGetConfigController" ).send_message( 0xabc, queue_get_config_request )
        controller( "QueueGetConfigController" ).should_receive( :queue_get_config_reply )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
