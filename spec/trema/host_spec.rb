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
require "trema/host"


module Trema
  describe Host do
    before {
      Host.instances.clear

      @cli = mock( "cli" )
      Cli.stub!( :new ).and_return( @cli )
    }


    describe :ip do
      context "when ip is omitted" do
        before { @stanza = {} }

        subject { Host.new( @stanza ).ip }

        it { should == "192.168.0.1" }
      end


      context "when ip \"192.168.100.100\"" do
        before { @stanza = { :ip => "192.168.100.100" } }

        subject { Host.new( @stanza ).ip }

        it { should == "192.168.100.100" }
      end
    end


    describe :mac do
      context "when mac is omitted" do
        before { @stanza = {} }

        subject { Host.new( @stanza ).mac }

        it { should == "00:00:00:00:00:01" }
      end


      context "when mac \"00:00:00:aa:bb:cc\"" do
        before { @stanza = { :mac => "00:00:00:aa:bb:cc" } }

        subject { Host.new( @stanza ).mac }

        it { should == "00:00:00:aa:bb:cc" }
      end
    end


    describe :netmask do
      context "when netmask is omitted" do
        before { @stanza = {} }

        subject { Host.new( @stanza ).netmask }

        it { should == "255.255.255.255" }
      end


      context "when netmask \"255.255.0.0\"" do
        before { @stanza = { :netmask => "255.255.0.0" } }

        subject { Host.new( @stanza ).netmask }

        it { should == "255.255.0.0" }
      end
    end


    context "when #add_arp_entries" do
      describe :cli do
        before {
          @host0 = Host.new( :name => "HOST 0" )
          @host1 = mock( "HOST 1" )
          @host2 = mock( "HOST 2" )
          @host3 = mock( "HOST 3" )
        }

        it "should add arp entries" do
          @cli.should_receive( :add_arp_entry ).with( @host1 )
          @cli.should_receive( :add_arp_entry ).with( @host2 )
          @cli.should_receive( :add_arp_entry ).with( @host3 )

          @host0.add_arp_entry [ @host1, @host2, @host3 ]
        end
      end
    end


    context "when #run!" do
      describe :cli do
        before {
          Phost.stub!( :new ).and_return( mock( "phost", :run! => nil ) )
        }

        it "should set IP and MAC address" do
          @cli.should_receive( :set_ip_and_mac_address )

          Host.new( :name => "Yutaro's host" ).run!
        end


        context "when promisc on" do
          before { @cli.stub!( :set_ip_and_mac_address ) }

          it "should enable promisc" do
            @cli.should_receive( :enable_promisc )

            Host.new( :name => "Yutaro's host", :promisc => true ).run!
          end
        end
      end
    end


    context "when #send_packets" do
      describe :cli do
        before {
          @dest = mock( "dest" )
          @options = mock( "options" )
        }

        it "should send_packets" do
          @cli.should_receive( :send_packets ).with( @dest, @options )

          Host.new( :name => "Yutaro's host" ).send_packet @dest, @options
        end
      end
    end


    context "when getting stats" do
      before {
        @stats = mock( "stats" )
        @host = Host.new( :name => "Yutaro's host" )
      }


      context "when #tx_stats" do
        describe :cli do
          it "should get tx stats" do
            @cli.should_receive( :tx_stats ).and_return( @stats )

            expect( @host.tx_stats ).to eq( @stats )
          end
        end
      end


      context "when #rx_stats" do
        describe :cli do
          it "should get rx stats" do
            @cli.should_receive( :rx_stats ).and_return( @stats )

            expect( @host.rx_stats ).to eq( @stats )
          end
        end
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
