#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
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
require "trema/mac"


module Trema
  describe Mac do
    before :each do
      @mac0 = Mac.new( 0 )
      @mac_str = Mac.new( "11:22:33:44:55:66" )
      @mac_max = Mac.new( 0xffffffffffff )
    end


    it "should be created from an integer value" do
      @mac0.value.should == 0
      @mac0.to_s.should == "00:00:00:00:00:00"

      @mac_max.value.should == 0xffffffffffff
      @mac_max.to_s.should == "ff:ff:ff:ff:ff:ff"
    end


    it "should be created from a string value" do
      @mac_str.value.should == "11:22:33:44:55:66"
      @mac_str.to_s.should == "11:22:33:44:55:66"
    end


    it "should be compared by its value" do
      @mac0.should == Mac.new( 0 )
      @mac_max.should == Mac.new( 0xffffffffffff )
    end


    it "should respond to #to_short and return an array of integer values" do
      @mac_str.to_short.should == [ 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 ]
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
