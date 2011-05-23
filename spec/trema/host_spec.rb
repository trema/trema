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
      @phost = mock( "phost" )
      Phost.stub!( :new ).and_return( @phost )
      @cli = mock( "cli" )
      Cli.stub!( :new ).and_return( @cli )

      @host = Host.new( :promisc => "on" )
    end


    it "should add arp entries" do
      host1 = mock( "HOST 1" )
      host2 = mock( "HOST 2" )
      host3 = mock( "HOST 3" )
      @cli.should_receive( :add_arp_entry ).once.with( host1 )
      @cli.should_receive( :add_arp_entry ).once.with( host2 )
      @cli.should_receive( :add_arp_entry ).once.with( host3 )

      @host.add_arp_entry [ host1, host2, host3 ]
    end


    it "should run phost and set network options" do
      @phost.should_receive( :run ).once.ordered
      @cli.should_receive( :set_ip_and_mac_address ).once.ordered
      @cli.should_receive( :enable_promisc ).once.ordered

      @host.run
    end


    it "should send packets" do
      dest = mock( "dest" )
      @cli.should_receive( :send_packets ).with( dest, anything )

      @host.send_packet dest
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
