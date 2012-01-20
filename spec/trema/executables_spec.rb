#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
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
require "trema/executables"


describe Trema::Executables do
  it "should know the path of tremashark executable" do
    Trema::Executables.should respond_to( :tremashark )
  end


  it "should know the path of switch_manager executable" do
    Trema::Executables.should respond_to( :switch_manager )
  end


  it "should know the path of switch_manager executable" do
    Trema::Executables.should respond_to( :switch_manager )
  end


  it "should know the path of packetin_filter executable" do
    Trema::Executables.should respond_to( :packetin_filter )
  end


  it "should know the path of phost executable" do
    Trema::Executables.should respond_to( :phost )
  end


  it "should know the path of phost's cli executable" do
    Trema::Executables.should respond_to( :cli )
  end


  it "should know the path of ovs-openflowd executable" do
    Trema::Executables.should respond_to( :ovs_openflowd )
  end


  it "should detect if all executables are compled or not" do
    FileTest.stub!( :executable? ).and_return( true )
    Trema::Executables.compiled?.should be_true

    FileTest.stub!( :executable? ).and_return( false )
    Trema::Executables.compiled?.should be_false
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
