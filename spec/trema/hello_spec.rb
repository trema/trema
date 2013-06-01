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
  describe Hello, ".new" do
    it_should_behave_like "any Openflow message with default transaction ID"
  end


  describe Hello, ".new(nil)" do
    subject { Hello.new( nil ) }
    it_should_behave_like "any Openflow message with default transaction ID"
  end


  describe Hello, ".new(transaction_id)" do
    subject { Hello.new( transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe Hello, ".new(:transaction_id => value)" do
    subject { Hello.new( :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe Hello, ".new(:xid => value)" do
    subject { Hello.new( :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"
  end


  describe Hello, '.new("INVALID OPTION")' do
    it { expect { Hello.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
