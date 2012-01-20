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


require "trema/app"
require "trema/host"
require "trema/link"
require "trema/packetin-filter"
require "trema/switch"
require "trema/switch-manager"


module Trema
  module DSL
    #
    # The current context of Trema DSL.
    #
    class Context
      #
      # set/get the port number for switch manager to listen to
      #
      # @example
      #   context.port = 5432
      #
      # @return [Number]
      #
      # @api public
      #
      attr_accessor :port

      #
      # use tremashark?
      #
      # @example
      #   context.tremashark = true
      #
      # @return [Boolean]
      #
      # @api public
      #
      attr_accessor :tremashark

      #
      # the hash of {App}
      #
      # @example
      #   p context.apps
      #   #=> {"trema tetris"=>#<Trema::App:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      # @api public
      #
      attr_reader :apps

      #
      # the hash of {Host}
      #
      # @example
      #   p context.hosts
      #   #=> {"host #0"=>#<Trema::Host:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      # @api public
      #
      attr_reader :hosts

      #
      # the hash of {Link}
      #
      # @example
      #   p context.links
      #   #=> {"link #0"=>#<Trema::Link:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      # @api public
      #
      attr_reader :links

      #
      # the hash of {Switch}
      #
      # @example
      #   p context.switches
      #   #=> {"switch #0"=>#<Trema::Switch:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      # @api public
      #
      attr_reader :switches


      #
      # Creates a new Trema DSL context
      #
      # @example
      #   context = Trema::DSL::Context.new
      #
      # @return [Context]
      #
      # @api public
      #
      def initialize
        @port = 6633
        @tremashark = false
        @apps = Trema::App.clear
        @hosts = Trema::Host.clear
        @links = Trema::Link.clear
        @packetin_filter = Trema::PacketinFilter.clear
        @switch_manager = Trema::SwitchManager.clear
        @switches = Trema::Switch.clear
      end


      #
      # Returns {PacketinFilter} configuration
      #
      # @example
      #   context.packetin_filter => #<Trema::PacketinFilter:0xb73c9328>
      #
      # @return [PacketinFilter]
      #
      # @api public
      #
      def packetin_filter
        @packetin_filter.values.last
      end


      #
      # Returns {SwitchManager} configuration
      #
      # @example
      #   context.switch_manager => #<Trema::SwitchManager:0xb73c9328>
      #
      # @return [SwitchManager]
      #
      # @api public
      #
      def switch_manager
        @switch_manager.values.last
      end


      #
      # Dumps a {Context} object to <code>file_name</code>
      #
      # @example
      #   context.dump_to "/tmp/.trema_session.dump"
      #
      # @return [Context]
      #
      # @api public
      #
      def dump_to file_name
        File.open( file_name, "w" ) do | f |
          f.print Marshal.dump( self )
        end
        self
      end


      #
      # Loads a {Context} object from <code>file_name</code>
      #
      # @example
      #   context.load_from "/tmp/.trema_session.dump"
      #
      # @return [Context]
      #
      # @api public
      #
      def load_from file_name
        if FileTest.exists?( file_name )
          Marshal.load( IO.read file_name )
        else
          self.class.new
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
