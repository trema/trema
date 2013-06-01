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
  describe FlowMod, ".new", :nosudo => true do
    it_should_behave_like "any Openflow message with default transaction ID"
  end


  describe FlowMod, ".new(nil)", :nosudo => true do
    subject { FlowMod.new( nil ) }
    it_should_behave_like "any Openflow message with default transaction ID"
  end


  describe FlowMod, ".new(transaction_id)", :nosudo => true do
    subject { FlowMod.new( transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe FlowMod, ".new(:transaction_id => value)", :nosudo => true do
    subject { FlowMod.new( :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe FlowMod, ".new(:xid => value)", :nosudo => true do
    subject { FlowMod.new( :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"
  end


  describe FlowMod, ".new(:command => value)", :nosudo => true do
    subject { FlowMod.new( :command => command ) }

    context "command: 0", :nosudo => true do
      let( :command ) { 0 }
      its( :command ) { should == 0 }
    end

    # TODO: boundary test.
  end


  describe FlowMod, ".new(:idle_timeout => value)", :nosudo => true do
    subject { FlowMod.new( :idle_timeout => idle_timeout ) }

    context "idle_timeout: 0", :nosudo => true do
      let( :idle_timeout ) { 0 }
      its( :idle_timeout ) { should == 0 }
    end

    # TODO: boundary test.
  end


  describe FlowMod, ".new(:hard_timeout => value)", :nosudo => true do
    subject { FlowMod.new( :hard_timeout => hard_timeout ) }

    context "hard_timeout: 0", :nosudo => true do
      let( :hard_timeout ) { 0 }
      its( :hard_timeout ) { should == 0 }
    end

    # TODO: boundary test.
  end


  describe FlowMod, '.new("INVALID OPTION")', :nosudo => true do
    it { expect { FlowMod.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
