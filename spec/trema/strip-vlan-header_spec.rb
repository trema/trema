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


describe StripVlanHeader, :type => "actions" do
  context "when sending a Flow Mod with StripVlanHeader" do
    it "should insert a new flow entry with action (strip_vlan)" do
      class TestController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( TestController ) {
        controller( "TestController" ).send_flow_mod_add( 0xabc, :actions => subject )
        sleep 2
        expect( vswitch( "0xabc" ) ).to have( 1 ).flows
        expect( vswitch( "0xabc" ).flows[ 0 ].actions ).to eq( "strip_vlan" )
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
