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
require "trema/dsl/vswitch"


module Trema
  module DSL
    describe Vswitch do
      before :each do
        @vswitch = Vswitch.new
      end


      context %[when parsing "vswitch { ... }"] do
        it %[recognizes "dpid DATAPATH_ID" directive] do
          expect { @vswitch.dpid "0xabc" }.not_to raise_error
        end


        it %[recognizes "datapath_id DATAPATH_ID" directive] do
          expect { @vswitch.datapath_id "0xabc" }.not_to raise_error
        end


        it %[recognizes "ports PORT_NUMBERS" directive] do
          expect { @vswitch.ports "0-4" }.not_to raise_error
        end


        it %[recognizes "ip IP_ADDRESS" directive] do
          expect { @vswitch.ip "192.168.0.1" }.not_to raise_error
        end
      end


      context "when getting the attributes of a vswitch" do
        it "returns its dpid in long format" do
          @vswitch.dpid "0xabc"
          expect( @vswitch[ :dpid_long ] ).to eq( "0000000000000abc" )
        end


        it "returns its dpid in short format" do
          @vswitch.dpid "0xabc"
          expect( @vswitch[ :dpid_short ] ).to eq( "0xabc" )
        end


        it "returns its ip address" do
          @vswitch.ip "192.168.0.1"
          expect( @vswitch[ :ip ] ).to eq( "192.168.0.1" )
        end
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
