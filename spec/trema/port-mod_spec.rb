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


describe PortMod, ".new( VALID OPTIONS )" do
  subject {
    PortMod.new(
      :port_no => 2,
      :hw_addr => Mac::new( "11:22:33:44:55:66" ),
      :config => 1,
      :mask => 1,
      :advertise => 0
    )
  }
  its( :port_no ) { should == 2 }
  its( "hw_addr.to_s" ) { should eq "11:22:33:44:55:66" }
  its( :config ) { should == 1 }
  its( :mask ) { should == 1 }
  its( :advertise ) { should == 0 }
  it_should_behave_like "any Openflow message with default transaction ID"


  describe "hw_addr" do
    it "should be a Trema::Mac object" do
      expect( PortMod.new(
               :port_no => 2,
               :hw_addr => Mac::new( "11:22:33:44:55:66" ),
               :config => 1,
               :mask => 1,
               :advertise => 0
             ).hw_addr.to_s ).to eq( "11:22:33:44:55:66" )
    end


    it "should be a string('11:22:33:44:55')" do
      expect( PortMod.new(
               :port_no => 2,
               :hw_addr => "11:22:33:44:55:66",
               :config => 1,
               :mask => 1,
               :advertise => 0 ).hw_addr.to_s ).to eq( "11:22:33:44:55:66" )
    end


    it "should be a number(281474976710655)" do
      expect( PortMod.new(
               :port_no => 2,
               :hw_addr => 281474976710655,
               :config => 1,
               :mask => 1,
               :advertise => 0 ).hw_addr.to_s ).to eq( "ff:ff:ff:ff:ff:ff" )
    end


    it "should otherwise raise ArgumentError" do
      expect { PortMod.new(
                :port_no => 2,
                :hw_addr => Array.new( 1234 ),
                :config => 1,
                :mask => 1,
                :advertise => 0
               )
             }.to raise_error( ArgumentError )
    end
  end
end


describe PortMod, "new( MANDATORY OPTIONS MISSING )" do
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError, "Port no, hw_addr, config, mask, advertise are mandatory options" )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
