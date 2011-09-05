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
require "trema"


module Trema
  describe Controller do
    context "when an instance is created" do
      subject { Controller.new }

      it "should have OpenFlow constants" do
        Controller::OFPP_MAX.should == 0xff00
        Controller::OFPP_IN_PORT.should == 0xfff8
        Controller::OFPP_TABLE.should == 0xfff9
        Controller::OFPP_NORMAL.should == 0xfffa
        Controller::OFPP_FLOOD.should == 0xfffb
        Controller::OFPP_ALL.should == 0xfffc
        Controller::OFPP_CONTROLLER.should == 0xfffd
        Controller::OFPP_LOCAL.should == 0xfffe
        Controller::OFPP_NONE.should == 0xffff

        Controller::OFPPR_ADD.should == 0x0
        Controller::OFPPR_DELETE.should == 0x1
        Controller::OFPPR_MODIFY.should == 0x2
      end
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
          switch( "0xabc" ).should have( 1 ).flows
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
