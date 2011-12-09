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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )
require "trema"


describe Trema::Shell, ".vswitch" do
  context "executed without a shell" do
    before { @context = nil }


    it "should raise" do
      expect { vswitch { dpid "0xabc" } }.to raise_error( "Not in Trema shell" )
    end
  end


  context "executed within a shell" do
    before { @context = mock( "context", :port => 6633 ) }
    after { Trema::Switch[ "0xabc" ].shutdown! }


    it "should create a new vswitch if name given" do
      vswitch { dpid "0xabc" }

      Trema::Switch.should have( 1 ).switch
      Trema::Switch[ "0xabc" ].name.should == "0xabc"
      Trema::Switch[ "0xabc" ].dpid_short.should == "0xabc"
      Trema::Switch[ "0xabc" ].dpid_long.should == "0000000000000abc"
    end


    it "should create a new vswitch if dpid given" do
      vswitch "0xabc"

      Trema::Switch.should have( 1 ).switch
      Trema::Switch[ "0xabc" ].name.should == "0xabc"
      Trema::Switch[ "0xabc" ].dpid_short.should == "0xabc"
      Trema::Switch[ "0xabc" ].dpid_long.should == "0000000000000abc"
    end


    it "should raise if dpid not given" do
      expect { vswitch }.to raise_error( "No dpid given" )
    end


    it "should raise if the name is invalid and block not given" do
      expect { vswitch "INVALID_DPID" }.to raise_error( "Invalid dpid: INVALID_DPID" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
