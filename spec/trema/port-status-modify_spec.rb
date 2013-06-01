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


describe Trema::PortStatusModify do
  it_should_behave_like "port status message", :klass => Trema::PortStatusModify
end


module Trema
  class PortStatusController < Controller
    def features_reply dpid, message
      message.ports.select( &:up? ).each do | each |
        port_mod = PortMod.new(
          :port_no => each.number,
          :hw_addr => each.hw_addr,
          :config => Port::OFPPC_PORT_DOWN,
          :mask => Port::OFPPC_PORT_DOWN,
          :advertise => 0
        )
        send_message dpid, port_mod
      end
    end
  end


  describe Controller do
    context "when one port goes down" do
      it "should receive port_status (modify)" do
        network {
          vswitch { datapath_id 0xabc }
          vhost "host"
          link "host", "0xabc"
        }.run( PortStatusController ) {
          controller( "PortStatusController" ).should_receive( :port_status ).with do | dpid, message |
            expect( dpid ).to eq( 0xabc )
            expect( message ).to be_an_instance_of( PortStatusModify )
          end

          controller( "PortStatusController" ).send_message 0xabc, FeaturesRequest.new
          sleep 2  # FIXME: wait to receive port_status
        }
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
