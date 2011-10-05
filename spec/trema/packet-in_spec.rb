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


describe Trema::PacketIn do
  def send_and_wait
    send_packets "host1", "host2"
    sleep 2
  end
  
  
  class PacketInController < Controller; end

  
  context "when instance is created" do
    it "should have valid datapath_id" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message | 
          datapath_id.should == 0xabc
          message.datapath_id.should == 0xabc
        end
        send_and_wait
      }
    end
    
    
    it "should have user data" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message | 
           # packet_in expected to have data portion.
          message.total_len.should > 20
          message.data.should be_instance_of( String )
          message.buffered?.should be_false
        end
        send_and_wait
      }
    end

    
    it "should have user L2 information (macsa)" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost( "host1" ) { mac "00:00:00:00:00:01" }
        vhost( "host2" ) { mac "00:00:00:00:00:02" }
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.macsa.should be_instance_of( Trema::Mac )
          message.macsa.to_s.should == "00:00:00:00:00:01"
        end
        send_and_wait
      }
    end
    

    it "should have user L2 information (macda)" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost( "host1" ) { mac "00:00:00:00:00:01" }
        vhost( "host2" ) { mac "00:00:00:00:00:02" }
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.macda.should be_instance_of( Trema::Mac )
          message.macda.to_s.should == "00:00:00:00:00:02"
        end
        send_and_wait
      }
    end

    
    it "should have valid input port" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.in_port.should > 0
        end
        send_and_wait
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
