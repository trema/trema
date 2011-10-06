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


require "rubygems"
require "rspec"


shared_examples_for "any Openflow message with default transaction ID" do
  context "when its transaction ID is auto-generated" do
    its( :transaction_id ) { should be_a_kind_of( Integer ) }
    its( :transaction_id ) { should >= 0 }
  end
end


shared_examples_for "any OpenFlow message" do
  let( :uint32_max ) { 2 ** 32 - 1 }


  context "when its transaction ID has a negative value" do
    let( :transaction_id ) { -1234 }
    it "should raise" do
      expect { subject }.to raise_error( "Transaction ID must be an unsigned 32bit integer" )
    end
  end
  

  context "when its transaction ID is zero" do
    let( :transaction_id ) { 0 }
    its( :transaction_id ) { should == 0 }
  end


  context "when its transaction ID is 1234" do
    let( :transaction_id ) { 1234 }
    its( :transaction_id ) { should == 1234 }
  end


  context "when its transaction ID is UINT32_MAX" do
    let( :transaction_id ) { uint32_max }
    its( :transaction_id ) { should == uint32_max }
  end


  context "when its transaction ID is UINT32_MAX + 1" do
    let( :transaction_id ) { uint32_max + 1 }
    it "should raise" do
      expect { subject }.to raise_error( "Transaction ID must be an unsigned 32bit integer" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End: