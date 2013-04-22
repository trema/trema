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
  describe EchoReply, ".new" do
    it_should_behave_like "any Openflow message with default transaction ID"
    its( :user_data ) { should be_nil }
  end


  describe EchoReply, ".new(nil)" do
    subject { EchoReply.new( nil ) }
    it_should_behave_like "any Openflow message with default transaction ID"
    its( :user_data ) { should be_nil }
  end


  describe EchoReply, ".new(transaction_id)" do
    subject { EchoReply.new( transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe EchoReply, ".new(:transaction_id => value)" do
    subject { EchoReply.new( :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe EchoReply, ".new(:xid => value)" do
    subject { EchoReply.new( :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"
  end


  describe EchoReply, ".new(:user_data => value)" do
    subject { EchoReply.new( :user_data => user_data ) }
    it_should_behave_like "any Openflow message with user_data"
  end


  describe EchoReply, ".new(:transaction_id => value, :user_data => value)" do
    subject { EchoReply.new( :transaction_id => transaction_id, :user_data => user_data ) }

    context 'transaction_id: 123, user_data: "USER DATA"' do
      let( :transaction_id ) { 123 }
      let( :user_data ) { "USER DATA" }
      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
      its( :user_data ) { should == "USER DATA" }
    end
  end


  describe EchoReply, '.new("INVALID OPTION")' do
    it { expect { EchoReply.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
