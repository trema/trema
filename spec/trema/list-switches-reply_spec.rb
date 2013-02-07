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


describe Controller do
  context "when #list_switches_request is sent" do
    it "should receive #list_switches_reply" do
      class ListSwitchesController < Controller; end
      network {
        vswitch { datapath_id 0x1 }
        vswitch { datapath_id 0x2 }
        vswitch { datapath_id 0x3 }
      }.run( ListSwitchesController ) {
        controller( "ListSwitchesController" ).should_receive( :list_switches_reply ) do | dpids |
          expect( dpids ).to eq( [ 0x1, 0x2, 0x3 ] )
        end
        controller( "ListSwitchesController" ).send_list_switches_request
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
