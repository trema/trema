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
require "trema/mac"


module Trema
  describe Mac, ".new(invalid_value)" do
    subject { Mac.new( value ) }

    context %{when "INVALID MAC ADDRESS"} do
      let( :value ) { "INVALID MAC ADDRESS" }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end

    context "when -1" do
      let( :value ) { -1 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end

    context "when 0x1000000000000" do
      let( :value ) { 0x1000000000000 }
      it { expect { subject }.to raise_error( ArgumentError ) }
    end

    context "when [ 1, 2, 3 ]" do
      let( :value ) { [ 1, 2, 3 ] }
      it { expect { subject }.to raise_error( TypeError ) }
    end
  end


  describe Mac, ".new(value)" do
    subject { Mac.new( value ) }

    context %{when "11:22:33:44:55:66"} do
      let( :value ) { "11:22:33:44:55:66" }
      it { should == Mac.new( "11:22:33:44:55:66" ) }
      its( :value ) { should == 0x112233445566 }
      its( :to_s ) { should == "11:22:33:44:55:66" }
      its( :to_a ) { should == [ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 ] }
      its( :multicast? ){ should be_true }
      its( :broadcast? ){ should be_false }
    end

    context "when 0" do
      let( :value ) { 0 }
      it { should == Mac.new( 0 ) }
      its( :value ) { should == 0 }
      its( :to_s ) { should == "00:00:00:00:00:00" }
      its( :to_a ) { should == [ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ] }
      its( :multicast? ){ should be_false }
      its( :broadcast? ){ should be_false }
    end

    context "when 0xffffffffffff" do
      let( :value ) { 0xffffffffffff }
      it { should == Mac.new( 0xffffffffffff ) }
      its( :value ) { should == 0xffffffffffff }
      its( :to_s ) { should == "ff:ff:ff:ff:ff:ff" }
      its( :to_a ) { should == [ 0xff, 0xff, 0xff, 0xff, 0xff, 0xff ] }
      its( :multicast? ){ should be_true }
      its( :broadcast? ){ should be_true }
    end
  end


  describe Mac do
    context "when querying FDB" do
      it "can be used for Hash keys" do
        fdb = {}
        fdb[ Mac.new( "00:00:00:00:00:01" ) ] = "Port #1"
        expect( fdb[ Mac.new( "00:00:00:00:00:01" ) ] ).to eq( "Port #1" )
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
