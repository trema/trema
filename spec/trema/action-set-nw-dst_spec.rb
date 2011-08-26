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
require "trema/ip"


describe Trema::ActionSetNwDst do
  context "when an instance is created" do
    subject { Trema::ActionSetNwDst.new( IP.new( "192.168.1.1" ) ) }
    its( :nw_dst ) { should be_an_instance_of Trema::IP  }
    it { should respond_to( :to_s ) }
    it "should print its attributes" do
      subject.to_s.should == "#<Trema::ActionSetNwDst> nw_dst = 192.168.1.1"
    end
    it { should respond_to( :to_i ) }
    it "should respond to #to_i and return an Integer" do
      subject.nw_dst.to_i.should == 3232235777
    end
    
    
    it "should append its action to a list of actions" do
      openflow_actions = double( )
      subject.should_receive( :append ).with( openflow_actions )
      subject.append( openflow_actions )
    end

    
    context "when nw_dst is not supplied" do
      it "should raise an error" do
        lambda do
          Trema::ActionSetNwDst.new( )
        end.should raise_error ArgumentError
      end
    end
  
    
    context "when nw_dst is not an IP object" do
      it "should raise an error" do
        lambda do
          Trema::ActionSetNwDst.new( 1234 )
        end.should raise_error ArgumentError, /nw dst address should be an IP object/
      end
    end
  end
  
  
  context "when sending #flow_mod(add) with action set to nw dst" do
    it "should have a flow with action set to mod_nw_dst" do
      class FlowModAddController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FlowModAddController ) {
        controller( "FlowModAddController" ).send_flow_mod_add( 0xabc, 
          :actions => ActionSetNwDst.new( IP.new( "192.168.1.1" ) ) )
        switch( "0xabc" ).should have( 1 ).flows
        switch( "0xabc" ).flows[0].actions.should match( /mod_nw_dst:192.168.1.1/ ) 
      }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
