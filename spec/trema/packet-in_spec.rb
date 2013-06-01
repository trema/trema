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
          expect( datapath_id ).to eq( 0xabc )
          expect( message.datapath_id ).to eq( 0xabc )
          expect( message.in_port ).to be > 0
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
          expect( message.total_len ).to be  > 20
          expect( message.data ).to be_instance_of( String )
          expect( message.buffered? ).to be_false

          expect( message.macsa ).to be_instance_of( Trema::Mac )
          expect( message.macsa.to_s ).to eq( "00:00:00:00:00:01" )
          expect( message.macda ).to be_instance_of( Trema::Mac )
          expect( message.macda.to_s ).to eq( "00:00:00:00:00:02" )

          expect( message.eth_type ).to eq( 0x0800 )
          expect( message.ipv4? ).to be_true
          expect( message.ipv4_version ).to eq( 4 )
          expect( message.ipv4_saddr ).to be_instance_of( Trema::IP )
          expect( message.ipv4_saddr.to_s ).to eq( "192.168.1.1" )
          expect( message.ipv4_daddr ).to be_instance_of( Trema::IP )
          expect( message.ipv4_daddr.to_s ).to eq( "192.168.1.2" )
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
          expect( message.in_port ).to be > 0
          expect( message.vtag? ).to be_false
          expect( message.arp? ).to be_true
          expect( message.rarp? ).to be_false
          expect( message.ipv4? ).to be_false
          expect( message.lldp? ).to be_false
          expect( message.tcp? ).to be_false
          expect( message.udp? ).to be_false
          expect( message.icmpv4? ).to be_false
          expect( message.igmp? ).to be_false

          expect( message.arp_oper ).to eq( 2 )
          expect( message.arp_sha.to_s ).to eq( "00:00:00:00:00:01" )
          expect( message.arp_spa.to_s ).to eq( "192.168.0.1" )
          expect( message.arp_tha.to_s ).to eq( "00:00:00:00:00:02" )
          expect( message.arp_tpa.to_s ).to eq( "192.168.0.2" )
          expect( message.arp_request? ).to be_false
          expect( message.arp_reply? ).to be_true

          expect( message.rarp_oper ).to be_nil
          expect( message.rarp_sha ).to be_nil
          expect( message.rarp_spa ).to be_nil
          expect( message.rarp_tha ).to be_nil
          expect( message.rarp_tpa ).to be_nil
          expect( message.rarp_request? ).to be_false
          expect( message.rarp_reply? ).to be_false

          expect( message.vlan_tpid ).to be_nil
          expect( message.vlan_tci ).to be_nil
          expect( message.vlan_prio ).to be_nil
          expect( message.vlan_cfi ).to be_nil
          expect( message.vlan_vid ).to be_nil

          expect( message.ipv4_version ).to be_nil
          expect( message.ipv4_ihl ).to be_nil
          expect( message.ipv4_tos ).to be_nil
          expect( message.ipv4_tot_len ).to be_nil
          expect( message.ipv4_id ).to be_nil
          expect( message.ipv4_frag_off ).to be_nil
          expect( message.ipv4_ttl ).to be_nil
          expect( message.ipv4_protocol ).to be_nil
          expect( message.ipv4_checksum ).to be_nil
          expect( message.ipv4_saddr ).to be_nil
          expect( message.ipv4_daddr ).to be_nil

          expect( message.icmpv4_type ).to be_nil
          expect( message.icmpv4_code ).to be_nil
          expect( message.icmpv4_checksum ).to be_nil
          expect( message.icmpv4_id ).to be_nil
          expect( message.icmpv4_seq ).to be_nil
          expect( message.icmpv4_gateway ).to be_nil

          expect( message.igmp_type ).to be_nil
          expect( message.igmp_checksum ).to be_nil
          expect( message.igmp_group ).to be_nil

          expect( message.tcp_src_port ).to be_nil
          expect( message.tcp_dst_port ).to be_nil
          expect( message.tcp_seq_no ).to be_nil
          expect( message.tcp_ack_no ).to be_nil
          expect( message.tcp_offset ).to be_nil
          expect( message.tcp_flags ).to be_nil
          expect( message.tcp_window ).to be_nil
          expect( message.tcp_checksum ).to be_nil
          expect( message.tcp_urgent ).to be_nil

          expect( message.udp_src_port ).to be_nil
          expect( message.udp_dst_port ).to be_nil
          expect( message.udp_checksum ).to be_nil
          expect( message.udp_len ).to be_nil
        end

        controller( "PacketInSendController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_CONTROLLER )
        )
        sleep 2
      }
    end

    it "should have correct RARP packet fields" do
      network {
        vswitch( "packet-in" ) { datapath_id 0xabc }
        vhost "host1"
        vhost ( "host2" ) { mac "00:00:00:00:00:02" }
        link "host1", "packet-in"
        link "host2", "packet-in"
      }.run( PacketInSendController ) {
        data = [
          0xff, 0xff, 0xff, 0xff, 0xff, 0xff, # dst
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # src
          0x80, 0x35, # ether type
          # rarp
          0x00, 0x01, # hardware type
          0x08, 0x00, # protocol type
          0x06, # hardware address length
          0x04, # protocol address length
          0x00, 0x03, # operation
          0x00, 0x00, 0x00, 0x00, 0x00, 0x01, # sender hardware address
          0xc0, 0xa8, 0x00, 0x01, # sender protocol address
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, # target hardware address
          0x00, 0x00, 0x00, 0x00, # target protocol address
          # padding to 64 bytes
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00
        ].pack( "C*" )
        controller( "PacketInSendController" ).should_receive( :packet_in ) do | datapath_id, message |
          expect( message.in_port ).to be > 0
          expect( message.vtag? ).to be_false
          expect( message.arp? ).to be_false
          expect( message.rarp? ).to be_true
          expect( message.ipv4? ).to be_false
          expect( message.lldp? ).to be_false
          expect( message.tcp? ).to be_false
          expect( message.udp? ).to be_false
          expect( message.icmpv4? ).to be_false
          expect( message.igmp? ).to be_false

          expect( message.arp_oper ).to be_nil
          expect( message.arp_sha ).to be_nil
          expect( message.arp_spa ).to be_nil
          expect( message.arp_tha ).to be_nil
          expect( message.arp_tpa ).to be_nil
          expect( message.arp_request? ).to be_false
          expect( message.arp_reply? ).to be_false

          expect( message.rarp_oper ).to eq( 3 )
          expect( message.rarp_sha.to_s ).to eq( "00:00:00:00:00:01" )
          expect( message.rarp_spa.to_s ).to eq( "192.168.0.1" )
          expect( message.rarp_tha.to_s ).to eq( "00:00:00:00:00:02" )
          expect( message.rarp_tpa.to_s ).to eq( "0.0.0.0" )
          expect( message.rarp_request? ).to be_true
          expect( message.rarp_reply? ).to be_false

          expect( message.vlan_tpid ).to be_nil
          expect( message.vlan_tci ).to be_nil
          expect( message.vlan_prio ).to be_nil
          expect( message.vlan_cfi ).to be_nil
          expect( message.vlan_vid ).to be_nil

          expect( message.ipv4_version ).to be_nil
          expect( message.ipv4_ihl ).to be_nil
          expect( message.ipv4_tos ).to be_nil
          expect( message.ipv4_tot_len ).to be_nil
          expect( message.ipv4_id ).to be_nil
          expect( message.ipv4_frag_off ).to be_nil
          expect( message.ipv4_ttl ).to be_nil
          expect( message.ipv4_protocol ).to be_nil
          expect( message.ipv4_checksum ).to be_nil
          expect( message.ipv4_saddr ).to be_nil
          expect( message.ipv4_daddr ).to be_nil

          expect( message.icmpv4_type ).to be_nil
          expect( message.icmpv4_code ).to be_nil
          expect( message.icmpv4_checksum ).to be_nil
          expect( message.icmpv4_id ).to be_nil
          expect( message.icmpv4_seq ).to be_nil
          expect( message.icmpv4_gateway ).to be_nil

          expect( message.igmp_type ).to be_nil
          expect( message.igmp_checksum ).to be_nil
          expect( message.igmp_group ).to be_nil

          expect( message.tcp_src_port ).to be_nil
          expect( message.tcp_dst_port ).to be_nil
          expect( message.tcp_seq_no ).to be_nil
          expect( message.tcp_ack_no ).to be_nil
          expect( message.tcp_offset ).to be_nil
          expect( message.tcp_flags ).to be_nil
          expect( message.tcp_window ).to be_nil
          expect( message.tcp_checksum ).to be_nil
          expect( message.tcp_urgent ).to be_nil

          expect( message.udp_src_port ).to be_nil
          expect( message.udp_dst_port ).to be_nil
          expect( message.udp_checksum ).to be_nil
          expect( message.udp_len ).to be_nil
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
          expect( message.in_port ).to be > 0
          expect( message.vtag? ).to be_false
          expect( message.arp? ).to be_false
          expect( message.rarp? ).to be_false
          expect( message.ipv4? ).to be_true
          expect( message.lldp? ).to be_false
          expect( message.udp? ).to be_false
          expect( message.tcp? ).to be_true
          expect( message.icmpv4? ).to be_false
          expect( message.igmp? ).to be_false

          expect( message.ipv4_version ).to eq( 4 )
          expect( message.ipv4_ihl ).to eq( 5 )
          expect( message.ipv4_tos ).to eq( 0 )
          expect( message.ipv4_tot_len ).to eq( 0x28 )
          expect( message.ipv4_id ).to eq( 0 )
          expect( message.ipv4_frag_off ).to eq( 0 )
          expect( message.ipv4_ttl ).to eq( 0 )
          expect( message.ipv4_protocol ).to eq( 6 )
          expect( message.ipv4_checksum ).to eq( 0x397d )
          expect( message.ipv4_saddr.to_s ).to eq( "192.168.0.1" )
          expect( message.ipv4_daddr.to_s ).to eq( "192.168.0.2" )

          expect( message.tcp_src_port ).to eq( 1 )
          expect( message.tcp_dst_port ).to eq( 2 )
          expect( message.tcp_seq_no ).to eq( 0 )
          expect( message.tcp_ack_no ).to eq( 0 )
          expect( message.tcp_offset ).to eq( 5 )
          expect( message.tcp_flags ).to eq( 0 )
          expect( message.tcp_window ).to eq( 0 )
          expect( message.tcp_checksum ).to eq( 11910 ) # 0x2e86
          expect( message.tcp_urgent ).to eq( 0 )

          expect( message.arp_oper ).to be_nil
          expect( message.arp_sha ).to be_nil
          expect( message.arp_spa ).to be_nil
          expect( message.arp_tha ).to be_nil
          expect( message.arp_tpa ).to be_nil

          expect( message.rarp_oper ).to be_nil
          expect( message.rarp_sha ).to be_nil
          expect( message.rarp_spa ).to be_nil
          expect( message.rarp_tha ).to be_nil
          expect( message.rarp_tpa ).to be_nil
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
          expect( message.in_port ).to be > 0
          expect( message.vtag? ).to be_false
          expect( message.arp? ).to be_false
          expect( message.rarp? ).to be_false
          expect( message.ipv4? ).to be_true
          expect( message.lldp? ).to be_false
          expect( message.tcp? ).to be_false
          expect( message.udp? ).to be_true
          expect( message.icmpv4? ).to be_false
          expect( message.igmp? ).to be_false


          expect( message.ipv4_version ).to eq( 4 )
          expect( message.ipv4_ihl ).to eq( 5 )
          expect( message.ipv4_tos ).to eq( 0 )
          expect( message.ipv4_tot_len ).to eq( 0x32 )
          expect( message.ipv4_id ).to eq( 0 )
          expect( message.ipv4_frag_off ).to eq( 0 )
          expect( message.ipv4_ttl ).to eq( 0x40 )
          expect( message.ipv4_protocol ).to eq( 17 )
          expect( message.ipv4_checksum ).to eq( 0xf968 )
          expect( message.ipv4_saddr.to_s ).to eq( "192.168.0.1" )
          expect( message.ipv4_daddr.to_s ).to eq( "192.168.0.2" )

          expect( message.udp_src_port ).to eq( 1 )
          expect( message.udp_dst_port ).to eq( 2 )
          expect( message.udp_checksum ).to eq( 0 )
          expect( message.udp_len ).to eq( 0x1e )
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
          expect( message.in_port ).to be > 0
          expect( message.vtag? ).to be_true
          expect( message.arp? ).to be_false
          expect( message.rarp? ).to be_false
          expect( message.ipv4? ).to be_true
          expect( message.lldp? ).to be_false
          expect( message.udp? ).to be_false
          expect( message.tcp? ).to be_false
          expect( message.icmpv4? ).to be_true
          expect( message.igmp? ).to be_false

          expect( message.vlan_tpid ).to eq( 0x8100 )
          expect( message.vlan_tci ).to eq( 0x0f9f )
          expect( message.vlan_prio ).to eq( 0 )
          expect( message.vlan_cfi ).to eq( 0 )
          expect( message.vlan_vid ).to eq( 0xf9f )
          expect( message.eth_type ).to eq( 0x0800 )

          expect( message.ipv4_version ).to eq( 4 )
          expect( message.ipv4_ihl ).to eq( 5 )
          expect( message.ipv4_tos ).to eq( 0 )
          expect( message.ipv4_tot_len ).to eq( 0x003c )
          expect( message.ipv4_id ).to eq( 0x8c1b )
          expect( message.ipv4_frag_off ).to eq( 0 )
          expect( message.ipv4_ttl ).to eq( 0x80 )
          expect( message.ipv4_protocol ).to eq( 1 )
          expect( message.ipv4_checksum ).to eq( 0xed09 )
          expect( message.ipv4_saddr.to_s ).to eq( "192.168.32.74" )
          expect( message.ipv4_daddr.to_s ).to eq( "192.168.32.1" )

          expect( message.icmpv4_type ).to eq( 8 )
          expect( message.icmpv4_code ).to eq( 0 )
          expect( message.icmpv4_checksum ).to eq( 0xe95b )
          expect( message.icmpv4_id ).to eq( 0x0400 )
          expect( message.icmpv4_seq ).to eq( 0x6000 )

          expect( message.icmpv4_echo_reply? ).to be_false
          expect( message.icmpv4_dst_unreach? ).to be_false
          expect( message.icmpv4_redirect? ).to be_false
          expect( message.icmpv4_echo_request? ).to be_true
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
          expect( message.in_port ).to be > 0
          expect( message.vtag? ).to be_false
          expect( message.arp? ).to be_false
          expect( message.rarp? ).to be_false
          expect( message.ipv4? ).to be_true
          expect( message.lldp? ).to be_false
          expect( message.udp? ).to be_false
          expect( message.tcp? ).to be_false
          expect( message.icmpv4? ).to be_false
          expect( message.igmp? ).to be_true

          expect( message.igmp_membership_query? ).to be_true
          expect( message.igmp_v1_membership_report? ).to be_false
          expect( message.igmp_v2_membership_report? ).to be_false
          expect( message.igmp_v2_leave_group? ).to be_false
          expect( message.igmp_v3_membership_report? ).to be_false

          expect( message.ipv4_version ).to eq( 4 )
          expect( message.ipv4_ihl ).to eq( 6 )
          expect( message.ipv4_tos ).to eq( 0xc0 )
          expect( message.ipv4_tot_len ).to eq( 0x0020 )
          expect( message.ipv4_id ).to eq( 0x3aea )
          expect( message.ipv4_frag_off ).to eq( 0 )
          expect( message.ipv4_ttl ).to eq( 1 )
          expect( message.ipv4_protocol ).to eq( 2 )
          expect( message.ipv4_checksum ).to eq( 0xe458 )
          expect( message.ipv4_saddr.to_s ).to eq( "192.168.100.43" )
          expect( message.ipv4_daddr.to_s ).to eq( "224.0.0.1" )

          expect( message.igmp_type ).to eq( 0x11 )
          expect( message.igmp_checksum ).to eq( 0xee9b )
        end

        controller( "PacketInSendController" ).send_packet_out(
          0xabc,
          :data => data,
          :actions => Trema::ActionOutput.new( :port => Controller::OFPP_CONTROLLER )
        )
        sleep 2
      }
    end

    it "should have correct LLDP packet fields" do
      network {
        vswitch( "packet-in" ) { datapath_id 0xabc }
        vhost "host1"
        vhost "host2"
        link "host1", "packet-in"
        link "host2", "packet-in"
      }.run( PacketInSendController ) {
        data = [
          0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E, # dst
          0x00, 0x00, 0x00, 0x00, 0x00, 0x02, # src
          0x88, 0xcc, # ether type
          0x02, 0x05, 0x07, 0x30, 0x78, 0x65, 0x31, # Chasis ID
          0x04, 0x02, 0x07, 0x31, # Port ID
          0x06, 0x02, 0x00, 0xb4, # TTL
          0x00, 0x00, # EOF
          0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
          0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
          0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
          0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
          0xa5, 0xa5, 0xa5, 0xa5, 0xa5, 0xa5,
          0xa5, 0xa5, 0xa5, 0xa5, 0xa5
        ].pack( "C*" )
        controller( "PacketInSendController" ).should_receive( :packet_in ) do | datapath_id, message |
          expect( message.in_port ).to be > 0
          expect( message.vtag? ).to be_false
          expect( message.arp? ).to be_false
          expect( message.rarp? ).to be_false
          expect( message.ipv4? ).to be_false
          expect( message.lldp? ).to be_true
          expect( message.udp? ).to be_false
          expect( message.tcp? ).to be_false
          expect( message.icmpv4? ).to be_false
          expect( message.igmp? ).to be_false
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
