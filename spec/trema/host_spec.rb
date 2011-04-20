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


describe Host do
  before :each do
    @stanza = Trema::DSL::Vhost.new( "VIRTUAL HOST" )
    @stanza.ip "192.168.0.100"
    @stanza.netmask "255.255.255.0"
    @stanza.mac "00:00:00:01:00:10"
  end


  it "should add arp entries" do
    host = Host.new( @stanza )
    host.interface = "INTERFACE"

    other_host1 = mock( "OTHER HOST 1", :ip => "192.168.0.1", :mac => "00:00:00:01:00:01" )
    other_host2 = mock( "OTHER HOST 2", :ip => "192.168.0.2", :mac => "00:00:00:01:00:02" )
    other_host3 = mock( "OTHER HOST 3", :ip => "192.168.0.3", :mac => "00:00:00:01:00:03" )

    host.should_receive( :sh ).once.ordered.with( /cli -i INTERFACE add_arp_entry --ip_addr 192.168.0.1 --mac_addr 00:00:00:01:00:01$/ )
    host.should_receive( :sh ).once.ordered.with( /cli -i INTERFACE add_arp_entry --ip_addr 192.168.0.2 --mac_addr 00:00:00:01:00:02$/ )
    host.should_receive( :sh ).once.ordered.with( /cli -i INTERFACE add_arp_entry --ip_addr 192.168.0.3 --mac_addr 00:00:00:01:00:03$/ )

    host.add_arp_entry [ other_host1, other_host2, other_host3 ]
  end


  it "should run phost and cli command with proper options" do
    host = Host.new( @stanza )
    host.interface = "INTERFACE"

    host.should_receive( :sh ).once.ordered.with( /phost \-i INTERFACE \-D$/ )
    host.should_receive( :sleep ).once.ordered.with( 1 )
    host.should_receive( :sh ).once.ordered.with( /cli -i INTERFACE set_host_addr --ip_addr 192.168.0.100 --ip_mask 255.255.255.0 --mac_addr 00:00:00:01:00:10$/ )

    host.run
  end


  it "should raise if interface is not added" do
    lambda do
      Host.new( @stanza ).run
    end.should raise_error( "The link(s) for vhost 'VIRTUAL HOST' is not defined." )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
