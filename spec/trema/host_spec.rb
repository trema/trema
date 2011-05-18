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
require "trema/dsl/vhost"
require "trema/host"


module Trema
  describe Host do
    before :each do
      @cli = mock( "cli" )
      Cli.stub!( :new ).and_return( @cli )

      stanza = {
        :name => "VIRTUAL HOST",
        :promisc => "on",
        :ip => "192.168.0.100",
        :netmask => "255.255.255.0",
        :mac => "00:00:00:01:00:10",
      }

      @host = Host.new( stanza )
    end


    it "should add arp entries" do
      @host.interface = "INTERFACE"

      other_host1 = mock( "OTHER HOST 1" )
      other_host2 = mock( "OTHER HOST 2" )
      other_host3 = mock( "OTHER HOST 3" )

      @cli.should_receive( :add_arp_entry ).once.ordered.with( @host, other_host1 )
      @cli.should_receive( :add_arp_entry ).once.ordered.with( @host, other_host2 )
      @cli.should_receive( :add_arp_entry ).once.ordered.with( @host, other_host3 )

      @host.add_arp_entry [ other_host1, other_host2, other_host3 ]
    end


    it "should run phost and cli command with proper options" do
      FileTest.stub!( :exists? ).and_return( true )

      @host.interface = "INTERFACE"

      @host.should_receive( :sh ).once.ordered.with( /phost \-i INTERFACE \-D$/ )
      @cli.should_receive( :set_host_addr ).once.ordered.with( @host )
      @cli.should_receive( :enable_promisc ).once.ordered.with( @host )

      @host.run
    end


    it "should raise if interface is not added" do
      lambda do
        @host.run
      end.should raise_error( "The link(s) for vhost 'VIRTUAL HOST' is not defined." )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
