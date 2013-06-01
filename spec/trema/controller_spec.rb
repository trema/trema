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


module Trema
  describe Controller do
    context "when using OpenFlow constants" do
      subject { Controller.constants }

      it { should include "OFPP_MAX" }
      it { should include "OFPP_IN_PORT" }
      it { should include "OFPP_TABLE" }
      it { should include "OFPP_NORMAL" }
      it { should include "OFPP_FLOOD" }
      it { should include "OFPP_ALL" }
      it { should include "OFPP_CONTROLLER" }
      it { should include "OFPP_LOCAL" }
      it { should include "OFPP_NONE" }
    end


    context "when logging" do
      subject { Controller.new }

      it { should respond_to :critical }
      it { should respond_to :error }
      it { should respond_to :warn }
      it { should respond_to :notice }
      it { should respond_to :info }
      it { should respond_to :debug }
    end


    context "when sending flow_mod messages" do
      it "should send a flow_mod_add message" do
        class FlowModAddController < Controller; end

        network {
          vswitch { datapath_id 0xabc }
        }.run( FlowModAddController ) {
          controller( "FlowModAddController" ).send_flow_mod_add( 0xabc )
          sleep 20 # FIXME: wait to send_flow_mod_add
          expect( vswitch( "0xabc" ) ).to have( 1 ).flows
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
