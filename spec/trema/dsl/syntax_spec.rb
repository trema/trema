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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )
require "trema/dsl/syntax"


describe Trema::DSL::Syntax do
  before( :each ) do
    Trema::DSL::Parser.stub!( :dump )
    @config = mock( :port => 6633, :link_index => 0 )
    @syntax = Trema::DSL::Syntax.new( @config )
  end


  it "should recognize 'use_tremashark' directive" do
    @config.should_receive( :tremashark= ).with( an_instance_of( Tremashark ) ).once

    @syntax.instance_eval do
      use_tremashark
    end
  end


  it "should recognize 'port' directive" do
    @config.should_receive( :port= ).with( 1234 ).once

    @syntax.instance_eval do
      port 1234
    end
  end


  it "should recognize 'link' directive" do
    @config.should_receive( :add_link ).with( an_instance_of( ::Link ) ).once

    @syntax.instance_eval do
      link "PEER0", "PEER1"
    end
  end


  it "should recognize 'switch' directive" do
    Trema::Switch.should_receive( :add ).with( an_instance_of( OpenflowSwitch ) ).once

    @syntax.instance_eval do
      switch { }
    end
  end


  it "should recognize 'vswitch' directive" do
    Trema::Switch.should_receive( :add ).with( an_instance_of( OpenVswitch ) ).once

    @syntax.instance_eval do
      vswitch { }
    end
  end


  it "should recognize 'vhost' directive" do
    Trema::Host.should_receive( :add ).with( an_instance_of( Trema::Host ) ).once

    @syntax.instance_eval do
      vhost { }
    end
  end


  it "should recognize 'filter' directive" do
    @config.should_receive( :set_filter ).with( an_instance_of( PacketinFilter ) ).once

    @syntax.instance_eval do
      filter :lldp => "LLDP RULE", :packet_in => "PACKET-IN RULE"
    end
  end


  it "should recognize 'event' directive" do
    @config.should_receive( :set_switch_manager ).with( an_instance_of( SwitchManager ) ).once

    @syntax.instance_eval do
      event "RULE"
    end
  end


  it "should recognize 'app' directive" do
    @config.should_receive( :add_app ).with( an_instance_of( App ) ).once

    @syntax.instance_eval do
      app { }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:

