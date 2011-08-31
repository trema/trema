#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


describe VendorRequest do
  context "when an instance is created with no arguments" do  
    its( :transaction_id ) { should be_a_kind_of( Integer ) }
    its( :transaction_id ) { should >= 0 }
    its( :vendor ) { should == 0xccddeeff }
    its( :data ) { should have( 16 ).items  }
  end  

  
  context "when an instance is created with all arguments set" do
    subject {
      @vendor_data = "this is a test".unpack( "C*" )
      @vendor_request = VendorRequest.new( 1234, 0x5555, @vendor_data )
    }
    its( :transaction_id ) { should == 1234 }
    its( :vendor ) { should == 0x5555 }
    let( :vendor_data ) { subject.data[0...@vendor_data.length]}
    describe "data" do
      it %{ should == [ 116, 104, 105, 115, 32, 105, 115, 32, 97, 32, 116, 101, 115, 116 ] } do
        vendor_data.should == @vendor_data
      end
    end
  end
  
  
  context "when an instance is created with" do
    describe "negative transaction ID" do
      it "should raise an error" do
        expect {
          VendorRequest.new( -1234, 0x5555, "test".unpack( "C*" ) )
        }.to raise_error( ArgumentError )
      end
    end
    
    
    describe "vendor user data specified as a string" do
      it "should raise an error" do
        expect {
          VendorRequest.new( 1234, 0x5555, "test" )
        }.to raise_error( ArgumentError )
      end
    end
    
    
    describe "more than 16 bytes of user data" do
      vendor_data = "this is a long message....".unpack( "C*" )
      vendor_request = VendorRequest.new( 1234, 0x5555, vendor_data )
      it "should assign the first 16 bytes only" do
        vendor_request.data.should == vendor_data[ 0..15 ]
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
