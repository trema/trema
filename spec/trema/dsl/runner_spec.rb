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
require "trema/dsl/runner"
require "trema/ordered-hash"


module Trema
  module DSL
    describe Runner do
      before :each do
        ::Process.stub!( :fork ).and_yield
        ::Process.stub!( :waitpid )
        @switch_manager = mock( "switch manager", :run! => nil )
        SwitchManager.stub!( :new ).and_return( @switch_manager )
      end


      context "when running" do
        it "should run switch_manager" do
          @switch_manager.should_receive( :run! ).once

          context = mock(
            "context",
            :port => 6633,
            :tremashark => nil,
            :switch_manager => nil,
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => {},
            :apps => {},
            :netnss => {}
          )

          Runner.new( context ).run
        end


        it "should run packetin_filter" do
          packetin_filter = mock
          packetin_filter.should_receive( :run! ).once

          context = mock(
            "context",
            :tremashark => nil,
            :switch_manager => mock( "switch_manager", :run! => nil ),
            :packetin_filter => packetin_filter,
            :links => {},
            :hosts => {},
            :switches => {},
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new( context ).run
        end


        it "should create links" do
          link0 = mock( "link0" )
          link0.should_receive( :delete! ).once
          link0.should_receive( :enable! ).once

          link1 = mock( "link1" )
          link1.should_receive( :delete! ).once
          link1.should_receive( :enable! ).once

          link2 = mock( "link2" )
          link2.should_receive( :delete! ).once
          link2.should_receive( :enable! ).once

          context = mock(
            "context",
            :tremashark => nil,
            :switch_manager => mock( "switch manager", :run! => nil ),
            :packetin_filter => nil,
            :links => { "link0" => link0, "link1" => link1, "link2" => link2 },
            :hosts => {},
            :switches => {},
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new( context ).run
        end


        it "should run vhosts" do
          host0 = mock( "host0" )
          host1 = mock( "host1" )
          host2 = mock( "host2" )

          host0.should_receive( :run! ).once
          host0.should_receive( :add_arp_entry ).with do | arg |
            expect( arg.size ).to eq( 2 )
            expect( arg ).to include( host1 )
            expect( arg ).to include( host2 )
          end

          host1.should_receive( :run! ).once
          host1.should_receive( :add_arp_entry ).with do | arg |
            expect( arg.size ).to eq( 2 )
            expect( arg ).to include( host0 )
            expect( arg ).to include( host2 )
          end

          host2.should_receive( :run! ).once
          host2.should_receive( :add_arp_entry ).with do | arg |
            expect( arg.size ).to eq( 2 )
            expect( arg ).to include( host0 )
            expect( arg ).to include( host1 )
          end

          context = mock(
            "context",
            :tremashark => nil,
            :switch_manager => mock( "switch manager", :run! => nil ),
            :packetin_filter => nil,
            :links => {},
            :hosts => { "host0" => host0, "host1" => host1, "host2" => host2 },
            :switches => {},
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new( context ).run
        end


        it "should run switches" do
          switch0 = mock( "switch0" )
          switch0.should_receive( :run! ).once

          switch1 = mock( "switch1" )
          switch1.should_receive( :run! ).once

          switch2 = mock( "switch2" )
          switch2.should_receive( :run! ).once

          context = mock(
            "context",
            :tremashark => nil,
            :switch_manager => mock( "switch manager", :run! => nil ),
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => { "switch0" => switch0, "switch1" => switch1, "switch 2" => switch2 },
            :apps => {},
            :netnss => {},
            :port => 6633
          )

          Runner.new( context ).run
        end


        it "should run apps" do
          apps = OrderedHash.new

          app0 = mock( "app0", :name => "app0" )
          app0.should_receive( :daemonize! ).once.ordered
          apps[ "app0" ] = app0

          app1 = mock( "app1", :name => "app1" )
          app1.should_receive( :daemonize! ).once.ordered
          apps[ "app1" ] = app1

          app2 = mock( "app2", :name => "app2" )
          app2.should_receive( :run! ).once.ordered
          apps[ "app2" ] = app2

          context = mock(
            "context",
            :tremashark => nil,
            :switch_manager => mock( "switch manager", :run! => nil, :rule => {} ),
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => {},
            :netnss => {},
            :apps => apps
          )

          Runner.new( context ).run
        end


        it "should daemonize apps" do
          apps = OrderedHash.new

          app0 = mock( "app0" )
          app0.should_receive( :daemonize! ).once.ordered
          apps[ "app0" ] = app0

          app1 = mock( "app1" )
          app1.should_receive( :daemonize! ).once.ordered
          apps[ "app1" ] = app1

          app2 = mock( "app2", :name => "App2" )
          app2.should_receive( :daemonize! ).once.ordered
          apps[ "app2" ] = app2

          context = mock(
            "context",
            :tremashark => nil,
            :switch_manager => mock( "switch manager", :run! => nil, :rule => {} ),
            :packetin_filter => nil,
            :links => {},
            :hosts => {},
            :switches => {},
            :netnss => {},
            :apps => apps
          )

          Runner.new( context ).daemonize
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
