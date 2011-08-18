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


describe Trema::Hello do
  it "should automatically allocate a transaction ID" do
    hello = Hello.new
    hello.transaction_id.should be_a_kind_of( Integer )
    hello.transaction_id.should >= 0
  end


  it "should be created by specifying its transaction ID" do
    hello = Hello.new( 1234 )
    hello.transaction_id.should == 1234
  end
  
  
  context "when creating from a negative transaction ID(-1234)" do
    it "should raise an error" do
      lambda do 
        Hello.new( -1234 )
      end.should raise_error( "Transaction ID must be >= 0" )
    end
  end
  
  
  context "when #hello is sent after controller initialization" do
    it "should receive #error" do
      class HelloController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( HelloController ) {
        hello = Hello.new( 1234 )
        controller( "HelloController" ).send_message( 0xabc, hello )
        controller( "HelloController" ).should_receive( :openflow_error )
        sleep 1
      }
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
