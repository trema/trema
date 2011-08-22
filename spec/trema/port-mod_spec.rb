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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema"


describe Trema::PortMod do
  context "when an instance is created with valid arguments" do
    it "should have valid transaction_id" do
      PortMod.new(2, Mac::new( "11:22:33:44:55:66" ), 1, 1, 0 ).transaction_id.should >= 0
    end
  
  
    it "should have valid port_no" do
      PortMod.new(2, Mac::new( "11:22:33:44:55:66" ), 1, 1, 0 ).port_no.should == 2
    end

    
    describe "hw_addr" do
      it "should be a Mac object" do
        PortMod.new(2, Mac::new( "11:22:33:44:55:66" ), 1, 1, 0 ).hw_addr.to_s.should eq( "11:22:33:44:55:66" )
      end
    
    
      it "should be a string('11:22:33:44:55')" do
        PortMod.new(2, "11:22:33:44:55:66", 1, 1, 0 ).hw_addr.to_s.should eq( "11:22:33:44:55:66" )
      end
    
    
      it "should be a number(281474976710655)" do
        PortMod.new(2, 281474976710655, 1, 1, 0 ).hw_addr.to_s.should eq( "ff:ff:ff:ff:ff:ff" )
      end
    
    
      it "should otherwise raise an error" do
        lambda do
          PortMod.new(2, [ 1234 ], 1, 1, 0 )
        end.should raise_error ArgumentError
      end
    end
  
  
    it "should have valid config" do
      PortMod.new(2, Mac::new( "11:22:33:44:55:66" ), 1, 1, 0 ).config.should == 1
    end
  
  
    it "should have valid mask" do
      PortMod.new(2, Mac::new( "11:22:33:44:55:66" ), 1, 1, 0 ).mask.should == 1
    end
  
  
    it "should have valid advertise" do
      PortMod.new(2, Mac::new( "11:22:33:44:55:66" ), 1, 1, 0 ).advertise.should == 0
    end
  
  end
  
  
  context "when an instance created with no arguments" do
    it "should raise an error" do
      lambda do
        PortMod.new()
      end.should raise_error( "wrong number of arguments (0 for 5)" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
