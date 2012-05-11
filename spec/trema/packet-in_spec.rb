
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
    it "should have valid datapath_id and in_port" do
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
          message.in_port.should > 0
        end
        send_and_wait
      }
    end


    it "should have vaild user data" do
      network {
        vswitch( "test" ) { datapath_id 0xabc }
        vhost( "host1" ) { mac "00:00:00:00:00:01"
                           ip "192.168.1.1" }
        vhost( "host2" ) { mac "00:00:00:00:00:02"
                           ip "192.168.1.2" }
        link "test", "host1"
        link "test", "host2"
      }.run( PacketInController ) {
        controller( "PacketInController" ).should_receive( :packet_in ) do | datapath_id, message |
           # packet_in expected to have data portion.
          message.total_len.should > 20
          message.data.should be_instance_of( String )
          message.buffered?.should be_false

          message.macsa.should be_instance_of( Trema::Mac )
          message.macsa.to_s.should == "00:00:00:00:00:01"
          message.macda.should be_instance_of( Trema::Mac )
          message.macda.to_s.should == "00:00:00:00:00:02"

          message.eth_type.should == 0x0800
          message.ipv4?.should == true
          message.ipv4_version.should == 4
          message.ipv4_saddr.should be_instance_of( Trema::IP )
          message.ipv4_saddr.to_s.should == "192.168.1.1"
          message.ipv4_daddr.should be_instance_of( Trema::IP )
          message.ipv4_daddr.to_s.should == "192.168.1.2"
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
          message.vtag?.should be_false
          message.arp?.should be_true
          message.ipv4?.should be_false
          message.tcp?.should be_false
          message.udp?.should be_false
          message.icmpv4?.should be_false
          message.igmp?.should be_false

          message.arp_oper.should == 2
          message.arp_sha.to_s.should == "00:00:00:00:00:01"
          message.arp_spa.to_s.should == "192.168.0.1"
          message.arp_tha.to_s.should == "00:00:00:00:00:02"
          message.arp_tpa.to_s.should == "192.168.0.2"
          message.arp_request?.should be_false
          message.arp_reply?.should be_true

          message.vlan_tpid.should be_nil
          message.vlan_tci.should be_nil
          message.vlan_prio.should be_nil
          message.vlan_cfi.should be_nil
          message.vlan_vid.should be_nil

          message.ipv4_version.should be_nil
          message.ipv4_ihl.should be_nil
          message.ipv4_tos.should be_nil
          message.ipv4_tot_len.should be_nil
          message.ipv4_id.should be_nil
          message.ipv4_frag_off.should be_nil
          message.ipv4_ttl.should be_nil
          message.ipv4_protocol.should be_nil
          message.ipv4_checksum.should be_nil
          message.ipv4_saddr.should be_nil
          message.ipv4_daddr.should be_nil

          message.icmpv4_type.should be_nil
          message.icmpv4_code.should be_nil
          message.icmpv4_checksum.should be_nil
          message.icmpv4_id.should be_nil
          message.icmpv4_seq.should be_nil
          message.icmpv4_gateway.should be_nil

          message.igmp_type.should be_nil
          message.igmp_checksum.should be_nil
          message.igmp_group.should be_nil

          message.tcp_src_port.should be_nil
          message.tcp_dst_port.should be_nil
          message.tcp_seq_no.should be_nil
          message.tcp_ack_no.should be_nil
          message.tcp_offset.should be_nil
          message.tcp_flags.should be_nil
          message.tcp_window.should be_nil
          message.tcp_checksum.should be_nil
          message.tcp_urgent.should be_nil

          message.udp_src_port.should be_nil
          message.udp_dst_port.should be_nil
          message.udp_checksum.should be_nil
          message.udp_len.should be_nil
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
          message.vtag?.should be_false
          message.arp?.should be_false
          message.ipv4?.should be_true
          message.udp?.should be_false
          message.tcp?.should be_true
          message.icmpv4?.should be_false
          message.igmp?.should be_false

          message.ipv4_version.should == 4
          message.ipv4_ihl.should == 5
          message.ipv4_tos.should == 0
          message.ipv4_tot_len.should == 0x28
          message.ipv4_id.should == 0
          message.ipv4_frag_off.should == 0
          message.ipv4_ttl.should == 0
          message.ipv4_protocol.should == 6
          message.ipv4_checksum.should == 0x397d
          message.ipv4_saddr.to_s.should == "192.168.0.1"
          message.ipv4_daddr.to_s.should == "192.168.0.2"

          message.tcp_src_port.should == 1
          message.tcp_dst_port.should == 2
          message.tcp_seq_no.should == 0
          message.tcp_ack_no.should == 0
          message.tcp_offset.should == 5
          message.tcp_flags.should == 0
          message.tcp_window.should == 0
          message.tcp_checksum.should == 11910 # 0x2e86
          message.tcp_urgent.should == 0

          message.arp_oper.should be_nil
          message.arp_sha.should be_nil
          message.arp_spa.should be_nil
          message.arp_tha.should be_nil
          message.arp_tpa.should be_nil
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
          message.vtag?.should be_false
          message.arp?.should be_false
          message.ipv4?.should be_true
          message.tcp?.should be_false
          message.udp?.should be_true
          message.icmpv4?.should be_false
          message.igmp?.should be_false

          message.ipv4_version.should == 4
          message.ipv4_ihl.should == 5
          message.ipv4_tos.should == 0
          message.ipv4_tot_len.should == 0x32
          message.ipv4_id.should == 0
          message.ipv4_frag_off.should == 0
          message.ipv4_ttl.should == 0x40
          message.ipv4_protocol.should == 17
          message.ipv4_checksum.should == 0xf968
          message.ipv4_saddr.to_s.should == "192.168.0.1"
          message.ipv4_daddr.to_s.should == "192.168.0.2"

          message.udp_src_port.should == 1
          message.udp_dst_port.should == 2
          message.udp_checksum.should == 0
          message.udp_len.should == 0x1e
        end

        controller( "PacketInSendController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_CONTROLLER )
        )
        sleep 2
      }
    end

    it "should have correct VLAN and ICMPv4 packet fields" do
      network {
        vswitch( "packet-in" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) {
          ip "192.168.32.1"
          netmask "255.255.255.0"
          mac "00:00:00:00:00:02"
        }
        link "host1", "packet-in"
        link "host2", "packet-in"
      }.run( PacketInSendController ) {
        data = [
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, # dst
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # src
          # vlan tag
          0x81, 0x00, # tpid
          0x0f, 0x9f, # tci
          0x08, 0x00, # ether type
          # ipv4
          0x45, 0x00, # version
          0x00, 0x3c, # length
          0x8c, 0x1b,
          0x00, 0x00,
          0x80,       # ttl
          0x01,       # protocol
          0xed, 0x09, # checksum
          0xc0, 0xa8, 0x20, 0x4a, # src
          0xc0, 0xa8, 0x20, 0x01, # dst
          # icmp
          0x08, # type
          0x00, # code
          0xe9, 0x5b, # checksum
          0x04, 0x00, # id
          0x60, 0x00, # seq
          0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
          0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
          0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61,
          0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69
        ].pack( "C*" )
        controller( "PacketInSendController" ).should_receive( :packet_in ) do | datapath_id, message |
          message.in_port.should > 0
          message.vtag?.should be_true
          message.arp?.should be_false
          message.ipv4?.should be_true
          message.udp?.should be_false
          message.tcp?.should be_false
          message.icmpv4?.should be_true
          message.igmp?.should be_false

          message.vlan_tpid.should == 0x8100
          message.vlan_tci.should == 0x0f9f
          message.vlan_prio.should == 0
          message.vlan_cfi.should == 0
          message.vlan_vid.should == 0xf9f
          message.eth_type.should == 0x0800

          message.ipv4_version.should == 4
          message.ipv4_ihl.should == 5
          message.ipv4_tos.should == 0
          message.ipv4_tot_len.should == 0x003c
          message.ipv4_id.should == 0x8c1b
          message.ipv4_frag_off.should == 0
          message.ipv4_ttl.should == 0x80
          message.ipv4_protocol.should == 1
          message.ipv4_checksum.should == 0xed09
          message.ipv4_saddr.to_s.should == "192.168.32.74"
          message.ipv4_daddr.to_s.should == "192.168.32.1"

          message.icmpv4_type.should == 8
          message.icmpv4_code.should == 0
          message.icmpv4_checksum.should == 0xe95b
          message.icmpv4_id.should == 0x0400
          message.icmpv4_seq.should == 0x6000

          message.icmpv4_echo_reply?.should be_false
          message.icmpv4_dst_unreach?.should be_false
          message.icmpv4_redirect?.should be_false
          message.icmpv4_echo_request?.should be_true
        end

        controller( "PacketInSendController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_CONTROLLER )
        )
        sleep 2
      }
    end

    it "should have correct IGMP packet fields" do
      network {
        vswitch( "packet-in" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "packet-in"
        link "host2", "packet-in"
      }.run( PacketInSendController ) {
        data = [
          0x01, 0x00, 0x5e, 0x00, 0x00, 0x01, # dst
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # src
          0x08, 0x00, # ether type
          # ipv4
          0x46, 0xc0, # version
          0x00, 0x20, # length
          0x3a, 0xea,
          0x00, 0x00,
          0x01,       # ttl
          0x02,       # protocol
          0xe4, 0x58, # checksum
          0xc0, 0xa8, 0x64, 0x2b, # src
          0xe0, 0x00, 0x00, 0x01, # dst
          0x94, 0x04, 0x00, 0x00,
          # igmp
          0x11, # type
          0x64, # code
          0xee, 0x9b, # checksum
          0x00, 0x00, 0x00, 0x00,
          0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
          0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70,
          0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x61,
          0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69
        ].pack( "C*" )
        controller( "PacketInSendController" ).should_receive( :packet_in ) do | datapath_id, message |
          message.in_port.should > 0
          message.vtag?.should be_false
          message.arp?.should be_false
          message.ipv4?.should be_true
          message.udp?.should be_false
          message.tcp?.should be_false
          message.icmpv4?.should be_false
          message.igmp?.should be_true

          message.igmp_membership_query?.should be_true
          message.igmp_v1_membership_report?.should be_false
          message.igmp_v2_membership_report?.should be_false
          message.igmp_v2_leave_group?.should be_false
          message.igmp_v3_membership_report?.should be_false

          message.ipv4_version.should == 4
          message.ipv4_ihl.should == 6
          message.ipv4_tos.should == 0xc0
          message.ipv4_tot_len.should == 0x0020
          message.ipv4_id.should == 0x3aea
          message.ipv4_frag_off.should == 0
          message.ipv4_ttl.should == 1
          message.ipv4_protocol.should == 2
          message.ipv4_checksum.should == 0xe458
          message.ipv4_saddr.to_s.should == "192.168.100.43"
          message.ipv4_daddr.to_s.should == "224.0.0.1"

          message.igmp_type.should == 0x11
          message.igmp_checksum.should == 0xee9b
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
