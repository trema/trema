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
require "trema/open-vswitch"


module Trema
  describe OpenVswitch do
    context "when running a vswitch" do
      it "should execute ovs openflowd" do
        stanza = { :dpid_short => "0xabc", :dpid_long => "0000000000000abc", :ip => "127.0.0.1" }
        stanza.stub!( :name ).and_return( "0xabc" )
        vswitch = OpenVswitch.new( stanza, 1234 )

        vswitch.should_receive( :sh ).with( /ovs\-openflowd/ ).once
        vswitch.run!
      end
    end
  end


  describe OpenVswitch, %[dpid = "0xabc"] do
    subject {
      stanza = { :dpid_short => "0xabc", :dpid_long => "0000000000000abc", :ip => "127.0.0.1" }
      stanza.stub!( :name ).and_return( "0xabc" )
      OpenVswitch.new stanza, 1234
    }

    its( :name ) { should == "0xabc" }
    its( :dpid_short ) { should == "0xabc" }
    its( :dpid_long ) { should == "0000000000000abc" }
    its( :network_device ) { should == "vsw_0xabc" }
  end


  describe OpenVswitch, %[name = "Otosan Switch", dpid = "0xabc"] do
    subject {
      stanza = { :dpid_short => "0xabc", :dpid_long => "0000000000000abc", :ip => "127.0.0.1" }
      stanza.stub!( :name ).and_return( "Otosan Switch" )
      OpenVswitch.new stanza, 1234
    }

    its( :name ) { should == "Otosan Switch" }
    its( :dpid_short ) { should == "0xabc" }
    its( :dpid_long ) { should == "0000000000000abc" }
    its( :network_device ) { should == "vsw_0xabc" }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
