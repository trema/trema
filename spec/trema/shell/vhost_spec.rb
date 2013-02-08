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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )


describe Trema::Shell, ".vhost" do
  before {
    Trema::Host.clear
    $context = mock( "context", :dump => true )
  }


  it "should create a new vhost if name given" do
    Trema::Shell.vhost( "host1" )

    expect( Trema::Host ).to have( 1 ).host
    expect( Trema::Host[ "host1" ].name ).to eq( "host1" )
  end


  it "should take ip, netmask, promisc, and mac option" do
    Trema::Shell.vhost( "host1" ) {
      ip "192.168.100.1"
      netmask "255.255.255.0"
      promisc "on"
      mac "00:00:00:1:1:1"
    }

    expect( Trema::Host ).to have( 1 ).host
    expect( Trema::Host[ "host1" ].name ).to eq( "host1" )
    expect( Trema::Host[ "host1" ].ip ).to eq( "192.168.100.1" )
    expect( Trema::Host[ "host1" ].promisc ).to be_true
    expect( Trema::Host[ "host1" ].mac ).to eq( "00:00:00:1:1:1" )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
