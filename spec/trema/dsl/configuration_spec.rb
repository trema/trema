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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )
require "trema/dsl/configuration"


module Trema
  module DSL
    describe Configuration do
      before :each do
        @context = Configuration.new
      end


      context "when parsing trema configurations" do
        it "should remember apps" do
          @context.should have( 0 ).apps

          Trema::App.add mock( "app #0", :name => "app #0" )
          Trema::App.add mock( "app #1", :name => "app #1" )
          Trema::App.add mock( "app #2", :name => "app #2" )

          @context.should have( 3 ).apps

          @context.apps[ "app #0" ].name.should == "app #0"
          @context.apps[ "app #1" ].name.should == "app #1"
          @context.apps[ "app #2" ].name.should == "app #2"
        end


        it "should remember hosts" do
          @context.should have( 0 ).hosts

          Trema::Host.add mock( "host #0", :name => "host #0" )
          Trema::Host.add mock( "host #1", :name => "host #1" )
          Trema::Host.add mock( "host #2", :name => "host #2" )

          @context.should have( 3 ).hosts

          @context.hosts[ "host #0" ].name.should == "host #0"
          @context.hosts[ "host #1" ].name.should == "host #1"
          @context.hosts[ "host #2" ].name.should == "host #2"
        end


        it "should remember links" do
          @context.should have( 0 ).links

          Trema::Link.add mock( "link #0", :name => "link #0" )
          Trema::Link.add mock( "link #1", :name => "link #1" )
          Trema::Link.add mock( "link #2", :name => "link #2" )

          @context.should have( 3 ).links

          @context.links[ "link #0" ].name.should == "link #0"
          @context.links[ "link #1" ].name.should == "link #1"
          @context.links[ "link #2" ].name.should == "link #2"
        end


        it "should remember filter settings" do
          @context.packetin_filter.should be_nil

          packetin_filter = mock( "filter", :name => "filter" )
          Trema::PacketinFilter.add packetin_filter

          @context.packetin_filter.should == packetin_filter
        end


        it "should remember switch manager" do
          @context.switch_manager.should be_nil

          switch_manager = mock( "switch manager", :name => "switch manager" )
          Trema::SwitchManager.add switch_manager

          @context.switch_manager.should == switch_manager
        end


        it "should remember switches" do
          @context.should have( 0 ).switches

          Trema::OpenflowSwitch.add mock( "switch #0", :name => "switch #0" )
          Trema::OpenflowSwitch.add mock( "switch #1", :name => "switch #1" )
          Trema::OpenflowSwitch.add mock( "switch #2", :name => "switch #2" )

          @context.should have( 3 ).switches

          @context.switches[ "switch #0" ].name.should == "switch #0"
          @context.switches[ "switch #1" ].name.should == "switch #1"
          @context.switches[ "switch #2" ].name.should == "switch #2"
        end
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
