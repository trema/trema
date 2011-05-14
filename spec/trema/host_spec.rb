#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
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
require "trema/host"


module Trema
  describe Host do
    before :each do
      Host.instances.clear
      
      @phost = mock( "phost" )
      Phost.stub!( :new ).and_return( @phost )
      
      @cli = mock( "cli" )
      Cli.stub!( :new ).and_return( @cli )

      @stanza = mock( "stanza", :[] => "HOST 0" )      
    end


    context "when IP address empty" do
      before { @stanza = {} }

      subject { Host.new( @stanza ).ip }

      it { should == "192.168.0.1" }
    end
    

    it "should add arp entries" do
      host1 = mock( "HOST 1" )
      host2 = mock( "HOST 2" )
      host3 = mock( "HOST 3" )

      @cli.should_receive( :add_arp_entry ).once.with( host1 )
      @cli.should_receive( :add_arp_entry ).once.with( host2 )
      @cli.should_receive( :add_arp_entry ).once.with( host3 )

      Host.new( @stanza ).add_arp_entry [ host1, host2, host3 ]
    end


    it "should run phost and set network options" do
      @phost.should_receive( :run ).once.ordered
      @cli.should_receive( :set_ip_and_mac_address ).once.ordered
      @cli.should_receive( :enable_promisc ).once.ordered

      Host.new( :promisc => "on" ).run!
    end


    it "should send packets" do
      dest = mock( "dest" )
      options = mock( "options" )
      @cli.should_receive( :send_packets ).with( dest, options )

      Host.new( @stanza ).send_packet dest, options
    end


    it "should return tx stats" do
      stats = mock( "stats" )
      @cli.should_receive( :tx_stats ).and_return( stats )

      Host.new( @stanza ).tx_stats.should == stats
    end


    it "should return rx stats" do
      stats = mock( "stats" )
      @cli.should_receive( :rx_stats ).and_return( stats )

      Host.new( @stanza ).rx_stats.should == stats
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
