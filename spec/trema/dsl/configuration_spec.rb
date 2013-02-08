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
          expect( @context ).to have( 0 ).apps

          Trema::App.add mock( "app #0", :name => "app #0" )
          Trema::App.add mock( "app #1", :name => "app #1" )
          Trema::App.add mock( "app #2", :name => "app #2" )

          expect( @context ).to have( 3 ).apps

          expect( @context.apps[ "app #0" ].name ).to eq( "app #0" )
          expect( @context.apps[ "app #1" ].name ).to eq( "app #1" )
          expect( @context.apps[ "app #2" ].name ).to eq( "app #2" )
        end


        it "should remember hosts" do
          expect( @context ).to have( 0 ).hosts

          Trema::Host.add mock( "host #0", :name => "host #0" )
          Trema::Host.add mock( "host #1", :name => "host #1" )
          Trema::Host.add mock( "host #2", :name => "host #2" )

          expect( @context ).to have( 3 ).hosts

          expect( @context.hosts[ "host #0" ].name ).to eq( "host #0" )
          expect( @context.hosts[ "host #1" ].name ).to eq( "host #1" )
          expect( @context.hosts[ "host #2" ].name ).to eq( "host #2" )
        end


        it "should remember links" do
          expect( @context ).to have( 0 ).links

          Trema::Link.add mock( "link #0", :name => "link #0" )
          Trema::Link.add mock( "link #1", :name => "link #1" )
          Trema::Link.add mock( "link #2", :name => "link #2" )

          expect( @context ).to have( 3 ).links

          expect( @context.links[ "link #0" ].name ).to eq( "link #0" )
          expect( @context.links[ "link #1" ].name ).to eq( "link #1" )
          expect( @context.links[ "link #2" ].name ).to eq( "link #2" )
        end


        it "should remember filter settings" do
          expect( @context.packetin_filter ).to be_nil

          packetin_filter = mock( "filter", :name => "filter" )
          Trema::PacketinFilter.add packetin_filter

          expect( @context.packetin_filter ).to eq( packetin_filter )
        end


        it "should remember switch manager" do
          expect( @context.switch_manager ).to be_nil

          switch_manager = mock( "switch manager", :name => "switch manager" )
          Trema::SwitchManager.add switch_manager

          expect( @context.switch_manager ).to eq( switch_manager )
        end


        it "should remember switches" do
          expect( @context ).to have( 0 ).switches

          Trema::OpenflowSwitch.add mock( "switch #0", :name => "switch #0" )
          Trema::OpenflowSwitch.add mock( "switch #1", :name => "switch #1" )
          Trema::OpenflowSwitch.add mock( "switch #2", :name => "switch #2" )

          expect( @context ).to have( 3 ).switches

          expect( @context.switches[ "switch #0" ].name ).to eq( "switch #0" )
          expect( @context.switches[ "switch #1" ].name ).to eq( "switch #1" )
          expect( @context.switches[ "switch #2" ].name ).to eq( "switch #2" )
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
