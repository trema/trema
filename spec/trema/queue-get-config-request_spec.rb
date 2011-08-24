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
    its( :transaction_id ) { should  >= 0 }
    its( :port ) { should == 1 }
  end
  
  
  context "when an instance is created with arguments" do
    subject { QueueGetConfigRequest.new( 123, 2 ) }
    its( :transaction_id ) { should == 123 }
    its( :port) { should == 2 }
  end

  
  context "when an instance is created with invalid transaction_id" do
    it "should raise an error" do
      lambda do
        QueueGetConfigRequest.new( -1, 1 )
      end.should raise_error ArgumentError
    end
  end
  
  
  context "when #queue_get_config_request is sent" do
    it "should #queue_get_config_reply" do
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
