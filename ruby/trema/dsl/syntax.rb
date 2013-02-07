#
# The top-level definition of Trema network DSL.
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


require "trema/app"
require "trema/dsl/link"
require "trema/dsl/netns"
require "trema/dsl/rswitch"
require "trema/dsl/run"
require "trema/dsl/switch"
require "trema/dsl/vhost"
require "trema/dsl/vswitch"
require "trema/dsl/custom-switch"
require "trema/hardware-switch"
require "trema/host"
require "trema/link"
require "trema/monkey-patch/module"
require "trema/netns"
require "trema/open-vswitch"
require "trema/packetin-filter"
require "trema/ruby-switch"
require "trema/switch-manager"
require "trema/custom-switch"


module Trema
  module DSL
    class Syntax
      def initialize config
        @config = config
      end


      def port number
        @config.port = number
      end


      def link peer0, peer1
        stanza = Trema::DSL::Link.new( peer0, peer1 )
        Trema::Link.new( stanza )
      end


      def switch name = nil, &block
        stanza = Trema::DSL::Switch.new( name )
        stanza.instance_eval( &block )
        Trema::HardwareSwitch.new( stanza )
      end


      def vswitch name = nil, &block
        stanza = Trema::DSL::Vswitch.new( name )
        stanza.instance_eval( &block )
        Trema::OpenVswitch.new stanza, @config.port
      end


      def rswitch name = nil, &block
        stanza = Trema::DSL::Rswitch.new( name )
        stanza.instance_eval( &block )
        Trema::RubySwitch.new( stanza )
      end


      def custom_switch name = nil, &block
        stanza = Trema::DSL::CustomSwitch.new( name )
        stanza.instance_eval( &block )
        Trema::CustomSwitch.new stanza
      end


      def vhost name = nil, &block
        stanza = Trema::DSL::Vhost.new( name )
        stanza.instance_eval( &block ) if block
        Trema::Host.new( stanza )
      end


      def netns name, &block
        stanza = Trema::DSL::Netns.new( name )
        stanza.instance_eval( &block ) if block
        Trema::Netns.new( stanza )
      end


      def filter rule
        Trema::PacketinFilter.new( rule )
      end


      def event rule
        Trema::SwitchManager.new( rule, @config.port )
      end


      def run name = nil, &block
        stanza = Trema::DSL::Run.new( name )
        stanza.instance_eval( &block )
        Trema::App.new( stanza )
      end
      deprecate :app => :run
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
