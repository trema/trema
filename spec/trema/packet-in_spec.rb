#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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

  class PacketInSendController < Controller 
    def packet_in datapath_id, message
      send_flow_mod_add(
                        datapath_id,
                        :match => Match.from( message ),
                        :actions => Trema::ActionOutput.new( :port => 2 )
                        )
      send_packet_out(
                      datapath_id,
                      :packet_in => message,
                      :actions => Trema::ActionOutput.new( :port => 2 )
                      )
    end
  end
  
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


    it "should have user L3 information" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost( "host1" ) { ip "192.168.1.1" }
        vhost( "host2" ) { ip "192.168.1.2" }
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.eth_type.should == 0x0800
          message.ipv4?.should == true
          message.ipv4_version.should == 4
          message.ipv4_protocol == 17
          message.ipv4_saddr.should be_instance_of( Trema::IP )
          message.ipv4_saddr.to_s.should == "192.168.1.1"
          message.ipv4_daddr.should be_instance_of( Trema::IP )
          message.ipv4_daddr.to_s.should == "192.168.1.2"
        end
        send_and_wait
      }
    end


    it "should have user L4 information (udp)" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost( "host1" )
        vhost( "host2" )
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.udp?.should == true
          message.udp_src_port.should == 9000
          message.udp_dst_port.should == 9001
        end
        send_packets "host1", "host2", :tp_src => 9000, :tp_dst => 9001
        sleep 2
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

  context "when reading packet content" do
    it "should have correct ARP packet fields" do
      network {
        vswitch( "packet-in" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) { mac "00:00:00:00:00:02" }
        link "host1", "packet-in"
        link "host2", "packet-in"
      }.run( PacketInSendController ) {
        data = [
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, # dst
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # src
          0x08, 0x06, # ether type
          # arp
          0x00, 0x01, # hardware type
          0x08, 0x00, # protocol type
          0x06, # hardware address length
          0x04, # protocol address length
          0x00, 0x02, # operation
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # sender hardware address
          0xc0, 0xa8, 0x00, 0x01, # sender protocol address
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, # target hardware address
          0xc0, 0xa8, 0x00, 0x02, # target protocol address
          # padding to 64 bytes
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00
        ].pack( "C*" )
        controller( "PacketInSendController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.in_port.should > 0
          message.arp?.should be_true
          message.tcp?.should be_false
          message.ipv4?.should be_false
          message.udp?.should be_false

          message.arp_oper.should == 2
          message.arp_sha.to_s.should == "00:00:00:00:00:01"
          message.arp_spa.to_s.should == "192.168.0.1"
          message.arp_tha.to_s.should == "00:00:00:00:00:02"
          message.arp_tpa.to_s.should == "192.168.0.2"
        end

        controller( "PacketInSendController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_CONTROLLER )
        )
        sleep 2
      }
    end

    it "should have correct TCP packet fields" do
      network {
        vswitch( "packet-in" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) {
          ip "192.168.0.2"
          netmask "255.255.0.0"
          mac "00:00:00:00:00:02"
        }
        link "host1", "packet-in"
        link "host2", "packet-in"
      }.run( PacketInSendController ) {
        data = [
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, # dst
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # src
          0x08, 0x00, # ether type
          # ipv4
          0x45, 0x00, # version
          0x00, 0x28, # length
          0x00, 0x00,
          0x00, 0x00,
          0x00,       # ttl
          0x06,       # protocol
          0x39, 0x7d, # checksum
          0xc0, 0xa8, 0x00, 0x01, # src
          0xc0, 0xa8, 0x00, 0x02, # dst
          # tcp
          0x00, 0x01, # src port
          0x00, 0x02, # dst port
          0x00, 0x00, 0x00, 0x00, # sequence number
          0x00, 0x00, 0x00, 0x00, # acknowledgement number
          0x50, # data offset, 
          0x00, # flags
          0x00, 0x00, # window size
          0x2e, 0x86, # checksum
          0x00, 0x00, # urgent pointer
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ].pack( "C*" )
        controller( "PacketInSendController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.in_port.should > 0
          message.arp?.should be_false
          message.udp?.should be_false
          message.ipv4?.should be_true
          message.tcp?.should be_true

          message.ipv4_saddr.to_s.should == "192.168.0.1"
          message.ipv4_daddr.to_s.should == "192.168.0.2"
          message.tcp_src_port.should == 1
          message.tcp_dst_port.should == 2
        end

        controller( "PacketInSendController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_CONTROLLER )
        )
        sleep 2
      }
    end

    it "should have correct UDP packet fields" do
      network {
        vswitch( "packet-in" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) {
          ip "192.168.0.2"
          netmask "255.255.0.0"
          mac "00:00:00:00:00:02"
        }
        link "host1", "packet-in"
        link "host2", "packet-in"
      }.run( PacketInSendController ) {
        data = [
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, # dst
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # src
          0x08, 0x00, # ether type
          # ipv4
          0x45, 0x00, # version
          0x00, 0x32, # length
          0x00, 0x00,
          0x00, 0x00,
          0x40,       # ttl
          0x11,       # protocol
          0xf9, 0x68, # checksum
          0xc0, 0xa8, 0x00, 0x01, # src
          0xc0, 0xa8, 0x00, 0x02, # dst
          # udp
          0x00, 0x01, # src port
          0x00, 0x02, # dst port
          0x00, 0x1e, # length
          0x00, 0x00, # checksum
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ].pack( "C*" )
        controller( "PacketInSendController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.in_port.should > 0
          message.arp?.should be_false
          message.tcp?.should be_false
          message.ipv4?.should be_true
          message.udp?.should be_true

          message.ipv4_saddr.to_s.should == "192.168.0.1"
          message.ipv4_daddr.to_s.should == "192.168.0.2"
          message.udp_src_port.should == 1
          message.udp_dst_port.should == 2
        end

        controller( "PacketInSendController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_CONTROLLER )
        )
        sleep 2
      }
    end

  end

end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
