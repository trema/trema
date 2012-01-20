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


class PacketOutController < Controller 
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


describe "packet-out" do
  context "a controller instance" do
    it "should respond to #send_packet_out" do
      PacketOutController.new.should respond_to(:send_packet_out)
    end
  end
    
  
  context "when invoked with no datapath_id" do
    it "should raise an error" do
      lambda do
        PacketOutController.new.send_packet_out
      end.should raise_error("wrong number of arguments (0 for 1)")
    end
  end
  
  
  context "when #packet_in" do
    it "should #packet_out" do
      network {
        vswitch( "packet-out" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "packet-out"
        link "host2", "packet-out"
      }.run( PacketOutController ) {
        send_packets "host2", "host1"
        sleep 2
        vhost( "host1" ).rx_stats.n_pkts.should == 1
      }
    end
  end


  context "when data argument is string type" do
    it "should #packet_out" do
      network {
        vswitch( "packet-out" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) {
          ip "192.168.0.2"
          netmask "255.255.0.0"
          mac "00:00:00:00:00:02"
        }
        link "host1", "packet-out"
        link "host2", "packet-out"
      }.run( PacketOutController ) {
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
          0xf9, 0x67, # checksum
          0xc0, 0xa8, 0x00, 0x01, # src
          0xc0, 0xa8, 0x00, 0x02, # dst
          # udp
          0x00, 0x01, # src port
          0x00, 0x01, # dst port
          0x00, 0x1e, # length
          0x00, 0x00, # checksum
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00
        ].pack( "C*" )
        controller( "PacketOutController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => 1 )
	)
        sleep 2
        vhost( "host2" ).rx_stats.n_pkts.should == 1
      }
    end
  end

  context "when reading packet content" do
    it "should have correct ARP packet fields" do
      network {
        vswitch( "packet-out" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) { mac "00:00:00:00:00:02" }
        link "host1", "packet-out"
        link "host2", "packet-out"
      }.run( PacketOutController ) {
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
        controller( "PacketOutController" ).should_receive( :packet_in ) do | datapath_id, message | 
          message.in_port.should > 0
#           message.arp?.should be_true
#           message.tcp?.should be_false
#           message.ipv4?.should be_false
#           message.udp?.should be_false

          # message.arp_oper.should == 2
          # message.arp_sha.to_s.should == "00:00:00:00:00:01"
          # message.arp_spa.to_s.should == "192.168.0.1"
          # message.arp_spa.to_s.should == "00:00:00:00:00:02"
          # message.arp_tpa.to_s.should == "192.168.0.2"
        end

        controller( "PacketOutController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_TABLE )
	)
        sleep 2
      }
    end

    it "should have correct TCP packet fields" do
      network {
        vswitch( "packet-out" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) {
          ip "192.168.0.2"
          netmask "255.255.0.0"
          mac "00:00:00:00:00:02"
        }
        link "host1", "packet-out"
        link "host2", "packet-out"
      }.run( PacketOutController ) {
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
        controller( "PacketOutController" ).should_receive( :packet_in ) do | datapath_id, message | 
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

        controller( "PacketOutController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_TABLE )
	)
        sleep 2
      }
    end

    it "should have correct UDP packet fields" do
      network {
        vswitch( "packet-out" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) {
          ip "192.168.0.2"
          netmask "255.255.0.0"
          mac "00:00:00:00:00:02"
        }
        link "host1", "packet-out"
        link "host2", "packet-out"
      }.run( PacketOutController ) {
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
        controller( "PacketOutController" ).should_receive( :packet_in ) do | datapath_id, message | 
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

        controller( "PacketOutController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_TABLE )
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
