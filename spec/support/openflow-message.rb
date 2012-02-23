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


require "rubygems"
require "rspec"


shared_examples_for "any Openflow message with default transaction ID" do
  its( :transaction_id ) { should be_a_kind_of( Integer ) }
  its( :transaction_id ) { should be_unsigned_32bit }
end


shared_examples_for "any OpenFlow message" do | options |
  option = options[ :option ]
  name = options[ :name ]
  size = options[ :size ]
  case size
  when 8
    let( :uint_max ) { 2 ** 8 - 1 }
  when 16
    let( :uint_max ) { 2 ** 16 - 1 }
  when 32
    let( :uint_max ) { 2 ** 32 - 1 }
  end


  context "when its #{ name } is a negative value" do
    let( option ) { -1234 }
    it "should raise ArgumentError" do
      expect { subject }.to raise_error( "#{ name } must be an unsigned #{ size }-bit integer" )
    end
  end


  context "when its #{ name } is zero" do
    let( option ) { 0 }
    its( option ) { should == 0 }
  end


  context "when its #{ name } is 123" do
    let( option ) { 123 }
    its( option ) { should == 123 }
  end


  context "when its #{ name } is UINT#{ size }MAX" do
    let( option ) { uint_max }
    its( option ) { should == uint_max }
  end


  context "when its #{ name } is UINT#{ size }_MAX + 1" do
    let( option ) { uint_max + 1 }
    it "should raise ArgumentError" do
      expect { subject }.to raise_error( "#{ name } must be an unsigned #{ size }-bit integer" )
    end
  end
end


shared_examples_for "any OpenFlow message with port option" do
  it_should_behave_like "any OpenFlow message", :option => :port, :name => "Port", :size => 16
end


shared_examples_for "any OpenFlow message with transaction_id option" do
  it_should_behave_like "any OpenFlow message", :option => :transaction_id, :name => "Transaction ID", :size => 32
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
