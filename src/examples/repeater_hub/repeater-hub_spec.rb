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


require File.join( File.dirname( __FILE__ ), "..", "..", "..", "spec", "spec_helper" )
require File.join( File.dirname( __FILE__ ), "repeater-hub" )


describe RepeaterHub do
  around do | example |
    network {
      vswitch("switch") { datapath_id "0xabc" }

      vhost("host1") { promisc "on" }
      vhost("host2") { promisc "on" }
      vhost("host3") { promisc "on" }

      link "switch", "host1"
      link "switch", "host2"
      link "switch", "host3"
    }.run( RepeaterHub ) {
      example.run
    }
  end


  context "when host1 sends one packet to host2" do
    it "should #packet_in" do
      controller( "RepeaterHub" ).should_receive( :packet_in )

      send_packets "host1", "host2"
      sleep 2 # FIXME: wait to send_packets
    end


    it "should #send_flow_mod_add" do
      controller( "RepeaterHub" ).should_receive( :send_flow_mod_add )

      send_packets "host1", "host2"
      sleep 2 # FIXME: wait to send_packets
    end


    describe "switch" do
      before { send_packets "host1", "host2" }

      subject { vswitch( "switch" ) }

      it { should have( 1 ).flows }
      its( "flows.first.actions" ) { should == "FLOOD" }
    end


    describe "host" do
      before { send_packets "host1", "host2" }

      subject { vhost( host_name ) }

      context "RX stats of host2" do
        let( :host_name ) { "host2" }
        its( "rx_stats.n_pkts" ) { should == 1 }
      end

      context "RX stats of host3" do
        let( :host_name ) { "host3" }
        its( "rx_stats.n_pkts" ) { should == 1 }
      end
    end
  end


  context "when host1 sends one more packet after the first packet" do
    before { send_packets "host1", "host2" }

    it "should not #packet_in" do
      controller( "RepeaterHub" ).should_not_receive( :packet_in )

      send_packets "host1", "host2"
    end

    it "should not #send_flow_mod_add" do
      controller( "RepeaterHub" ).should_not_receive( :send_flow_mod_add )

      send_packets "host1", "host2"
    end


    describe "switch" do
      before { send_packets "host1", "host2" }

      subject { vswitch( "switch" ) }

      it { should have( 1 ).flows }
      its( "flows.first.actions" ) { should == "FLOOD" }
    end


    describe "host" do
      before { send_packets "host1", "host2" }

      subject { vhost( host_name ) }

      context "RX stats of host2" do
        let( :host_name ) { "host2" }
        its( "rx_stats.n_pkts" ) { should == 2 }
      end

      context "RX stats of host3" do
        let( :host_name ) { "host3" }
        its( "rx_stats.n_pkts" ) { should == 2 }
      end
    end
  end


  context "when host1 sends 100 packets to host2" do
    describe "host" do
      before { send_packets "host1", "host2", :pps => 100 }

      subject { vhost( host_name ) }

      context "RX stats of host2" do
        let( :host_name ) { "host2" }
        its( "rx_stats.n_pkts" ) { should == 100 }
      end

      context "RX stats of host3" do
        let( :host_name ) { "host3" }
        its( "rx_stats.n_pkts" ) { should == 100 }
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
