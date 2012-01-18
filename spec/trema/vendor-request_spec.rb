#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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


describe VendorRequest, ".new( OPTIONAL OPTION MISSING )" do
  its( :vendor ) { should == 0xccddeeff }
  its( :data ) { should have( 16 ).items  }
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe VendorRequest, ".new( VALID OPTIONS )" do
  subject {
    @vendor_data = "this is a test".unpack( "C*" )
    VendorRequest.new :vendor_id => 0x5555, :vendor_data => @vendor_data, :transaction_id => transaction_id
  }
  let( :transaction_id ) { 1234 }
  its( :vendor ) { should == 21845 }
  let( :vendor_data ) { subject.data[0...@vendor_data.length]}
  describe "data" do
    it %{ should == [ 116, 104, 105, 115, 32, 105, 115, 32, 97, 32, 116, 101, 115, 116 ] } do
      vendor_data.should == @vendor_data
    end
  end
  it_should_behave_like "any OpenFlow message with transaction_id option"
end


describe VendorRequest, ".new( INVALID OPTION ) - vendor_data as String" do
  subject { VendorRequest.new :vendor_data => "test" }
  it "should raise TypeError" do
    expect { subject }.to raise_error( TypeError )
  end
end


describe VendorRequest, ".new( INVALID OPTIONS )" do
  it "should raise TypeError" do
    expect {
      VendorRequest.new "INVALID OPTIONS"
    }.to raise_error( TypeError )
  end
end


describe VendorRequest, ".new( VALID OPTIONS ) - vendor_data greater than 16 bytes" do
  vendor_data = "this is a long message....".unpack( "C*" )
  subject { VendorRequest.new :vendor_data => vendor_data }
  it "should assign the first 16 bytes only" do
    subject.data.should == vendor_data[ 0..15 ]
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
