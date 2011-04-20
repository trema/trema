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
require "trema/dsl/link"
require "trema/link"


describe Link do
  before :each do
    @stanza = Trema::DSL::Link.new( "Virtual Host", "Virtual Switch" )
  end


  it "returns two network interfaces for each peer" do
    link = Link.new( @stanza, 0 )
    link.interfaces[ 0 ].should == "trema0-0"
    link.interfaces[ 1 ].should == "trema0-1"

    link = Link.new( @stanza, 1 )
    link.interfaces[ 0 ].should == "trema1-0"
    link.interfaces[ 1 ].should == "trema1-1"
  end


  context "when creating/deleting a link" do
    before :each do
      @link = Link.new( @stanza, 0 )
    end


    it "executes ip and ifconfig command" do
      @link.should_receive( :sh ).once.ordered.with( "sudo ip link add name trema0-0 type veth peer name trema0-1" )
      @link.should_receive( :sh ).once.with( "sudo /sbin/ifconfig trema0-0 up" )
      @link.should_receive( :sh ).once.with( "sudo /sbin/ifconfig trema0-1 up" )

      @link.up!
    end


    it "executes ip and ifconfig command" do
      @link.should_receive( :sh ).once.ordered.with( "sudo /sbin/ifconfig trema0-0 down 2>/dev/null" )
      @link.should_receive( :sh ).once.ordered.with( "sudo /sbin/ifconfig trema0-1 down 2>/dev/null" )
      @link.should_receive( :sh ).once.with( "sudo ip link delete trema0-0 2>/dev/null" )

      @link.down!
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
