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
      expect( PacketOutController.new ).to respond_to(:send_packet_out)
    end
  end


  context "when invoked with no datapath_id" do
    it "should raise an error" do
      expect { PacketOutController.new.send_packet_out }.to raise_error
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
        expect( vhost( "host1" ).rx_stats.n_pkts ).to eq( 1 )
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
        expect( vhost( "host2" ).rx_stats.n_pkts ).to eq( 1 )
      }
    end
  end

end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
