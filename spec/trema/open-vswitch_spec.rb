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
  describe OpenVswitch, %[dpid = "0xabc"] do
    before :each do
      @stanza = mock( "stanza", :name => "0xabc" )
      @stanza.stub!( :[] ).with( :dpid_short ).and_return( "0xabc" )
      @stanza.stub!( :[] ).with( :dpid_long ).and_return( "0000000000000abc" )
      @stanza.stub!( :get ).with( :dpid_short ).and_return( "0xabc" )
      @stanza.stub!( :get ).with( :ip ).and_return( "127.0.0.1" )
    end


    subject { OpenVswitch.new @stanza, 1234 }


    its( :name ) { should == "0xabc" }
    its( :dpid_short ) { should == "0xabc" }
    its( :dpid_long ) { should == "0000000000000abc" }
    its( :network_device ) { should == "vsw_0xabc" }


    context "when running a vswitch" do
      it "should execute ovs openflowd" do
        subject.should_receive( :sh ).with( /ovs\-openflowd/ ).once
        subject.run!
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
