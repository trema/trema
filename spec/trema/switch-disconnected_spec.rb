#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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
require "trema"


class SwitchDisconnected < Controller; end


describe SwitchDisconnected do 
  
  context "when switch is killed" do
    it "should receive a switch_disconnected" do
      network {
        vswitch( "switch-disconnect" ) { datapath_id 0xabc }
      }.run( SwitchDisconnected ) {
        controller( "SwitchDisconnected" ).should_receive( :switch_disconnected )
        switch( "switch-disconnect" ).shutdown!
      }
    end
    
    
    it "should receive a switch_disconnected with valid datapath_id" do
      network {
        vswitch( "switch-disconnect" ) { datapath_id 0xabc }
      }.run( SwitchDisconnected ) {
        controller( "SwitchDisconnected" ).should_receive( :switch_disconnected ).with( 0xabc )
        switch( "switch-disconnect" ).shutdown!
      }
    end
  end
  
  
end