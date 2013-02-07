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
  describe Vendor, ".new" do
    it_should_behave_like "any Openflow message with default transaction ID"
    its ( :vendor ) { should == 0 }
    its ( :data ) { should be_nil }
  end


  describe Vendor, ".new(nil)" do
    it_should_behave_like "any Openflow message with default transaction ID"
    its ( :vendor ) { should == 0 }
    its ( :data ) { should be_nil }
  end


  describe Vendor, ".new(:transaction_id => value)" do
    subject { Vendor.new( :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe Vendor, ".new(:xid => value)" do
    subject { Vendor.new( :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"
  end


  describe Vendor, ".new(:vendor_id => value)", :nosudo => true do
    subject { Vendor.new( :vendor => vendor ) }
    let( :vendor ) { 0xdeadbeef }
    its( :vendor ) { should == 0xdeadbeef }
    its ( :data ) { should be_nil }
  end


  describe Vendor, ".new(:data => value)", :nosudo => true do
    subject { Vendor.new( :data => data ) }
    let( :data ) { "VENDOR DATA".unpack( "C*" ) }
    its( :data ) { should == [86, 69, 78, 68, 79, 82, 32, 68, 65, 84, 65] }
    its ( :vendor ) { should == 0 }
  end


  describe Vendor, '.new("INVALID OPTION")', :nosudo => true do
    it { expect { Vendor.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
