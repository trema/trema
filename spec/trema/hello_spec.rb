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
  describe Hello, ".new( options )", :nosudo => true do
    subject { Hello.new( options ) }


    context "options: none" do
      subject { Hello.new }
      its( :transaction_id ) { should be_unsigned_32bit }
      its( :xid ) { should be_unsigned_32bit }
    end


    context "options: nil" do
      let( :options ) { nil }
      its( :transaction_id ) { should be_unsigned_32bit }
      its( :xid ) { should be_unsigned_32bit }
    end


    context "options: number" do
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


    context "options: :transaction_id => ..." do
      let( :options ) { { :transaction_id => transaction_id } }

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


    context "options: :xid => ..." do
      let( :options ) { { :xid => xid } }

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


    context 'options: "INVALID OPTIONS"' do
      let( :options ) { "INVALID OPTIONS" }
      it { expect { subject }.to raise_error( TypeError ) }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
