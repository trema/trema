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


module Trema
  describe Hello, :nosudo => true do
    its( :transaction_id ) { should be_unsigned_32bit }
    its( :xid ) { should be_unsigned_32bit }
  end


  describe Hello, ".new(nil)", :nosudo => true do
    subject { Hello.new( nil ) }
    its( :transaction_id ) { should be_unsigned_32bit }
    its( :xid ) { should be_unsigned_32bit }
  end


  describe Hello, ".new(transaction_id)", :nosudo => true do
    subject { Hello.new( transaction_id ) }

    context "transaction_id: -123" do
      let( :transaction_id ) { -123 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end

    context "transaction_id: 0" do
      let( :transaction_id ) { 0 }
      its( :transaction_id ) { should == 0 }
      its( :xid ) { should == 0 }
    end

    context "transaction_id: 123" do
      let( :transaction_id ) { 123 }
      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
    end

    context "transaction_id: UINT32_MAX" do
      let( :transaction_id ) { 2 ** 32 - 1 }
      its( :transaction_id ) { should == 2 ** 32 - 1 }
      its( :xid ) { should == 2 ** 32 - 1 }
    end

    context "transaction_id: UINT32_MAX + 1" do
      let( :transaction_id ) { 2 ** 32 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end
  end


  describe Hello, ".new(:transaction_id => value)", :nosudo => true do
    subject { Hello.new( :transaction_id => transaction_id ) }

    context "transaction_id: -123" do
      let( :transaction_id ) { -123 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end

    context "transaction_id: 0" do
      let( :transaction_id ) { 0 }
      its( :transaction_id ) { should == 0 }
      its( :xid ) { should == 0 }
    end

    context "transaction_id: 123" do
      let( :transaction_id ) { 123 }
      its( :transaction_id ) { should == 123 }
      its( :xid ) { should == 123 }
    end

    context "transaction_id: UINT32_MAX" do
      let( :transaction_id ) { 2 ** 32 - 1 }
      its( :transaction_id ) { should == 2 ** 32 - 1 }
      its( :xid ) { should == 2 ** 32 - 1 }
    end

    context "transaction_id: UINT32_MAX + 1" do
      let( :transaction_id ) { 2 ** 32 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end
  end


  describe Hello, ".new(:xid => value)", :nosudo => true do
    subject { Hello.new( :xid => xid ) }

    context "xid: -123" do
      let( :xid ) { -123 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end

    context "xid: 0" do
      let( :xid ) { 0 }
      its( :xid ) { should == 0 }
      its( :transaction_id ) { should == 0 }
    end

    context "xid: 123" do
      let( :xid ) { 123 }
      its( :xid ) { should == 123 }
      its( :transaction_id ) { should == 123 }
    end

    context "xid: UINT32_MAX" do
      let( :xid ) { 2 ** 32 - 1 }
      its( :xid ) { should == 2 ** 32 - 1 }
      its( :transaction_id ) { should == 2 ** 32 - 1 }
    end

    context "xid: UINT32_MAX + 1" do
      let( :xid ) { 2 ** 32 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end
  end


  describe Hello, '.new("INVALID OPTION")', :nosudo => true do
    it { expect { Hello.new "INVALID OPTION" }.to raise_error( TypeError ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
