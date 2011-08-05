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


describe Trema::ActionSetDlDst do
  context "when an instance is created" do
    it "should have a valid dl_dst attribute specified as a Trema::Mac object" do
      action_set_dl_dst = Trema::ActionSetDlDst.new( Mac.new( "52:54:00:a8:ad:8c" ) )
      action_set_dl_dst.dl_dst.should be_an_instance_of( Trema::Mac )
    end
  end
  
  
  it "should raise an error if its dl_dst attribute is not specified" do
    expect {
      action_set_dl_dst = Trema::ActionSetDlDst.new( )
    }.to raise_error( ArgumentError )
  end
  
  
  it "should raise an error if its dl_dst attribute is not a Mac object" do
    expect {
      action_set_dl_dst = Trema::ActionSetDlDst.new( 1234 )
    }.to raise_error( ArgumentError, /dl dst address should be a Mac object/ )
  end
  
  
  it "should respond to #to_s and return a string" do
    action_set_dl_dst = Trema::ActionSetDlDst.new( Mac.new( "11:22:33:44:55:66" ) )
    action_set_dl_dst.should respond_to( :to_s )
    action_set_dl_dst.to_s.should == "#<Trema::ActionSetDlDst> dl_dst = 11:22:33:44:55:66"
  end 
  
  
  it "should append its dl_dst attribute to a list of actions" do
    action_set_dl_dst = Trema::ActionSetDlDst.new( Mac.new( "52:54:00:a8:ad:8c" ) )
    openflow_actions = double( )
    action_set_dl_dst.should_receive( :append ).with( openflow_actions )
    action_set_dl_dst.append( openflow_actions )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
