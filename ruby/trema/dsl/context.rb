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
      attr_accessor :port  # @return [Number] the port number for switch manager to listen to
      attr_accessor :tremashark  # @return [Boolean] use tremashark?
      attr_reader :apps  # @return [Hash] the hash of apps
      attr_reader :hosts  # @return [Hash] the hash of hosts
      attr_reader :links  # @return [Hash] the hash of links
      attr_reader :switches  # @return [Hash] the hash of switches


      def initialize
        @port = 6633
        @tremashark = false
        @apps = Trema::App.instances.clear
        @hosts = Trema::Host.instances.clear
        @links = Trema::Link.instances.clear
        @packetin_filter = Trema::PacketinFilter.instances.clear
        @switch_manager = Trema::SwitchManager.instances.clear
        @switches = Trema::Switch.instances.clear
      end


      #
      # @return [PacketinFilter]
      #
      def packetin_filter
        @packetin_filter.values.last
      end


      #
      # @return [SwitchManager]
      #
      def switch_manager
        @switch_manager.values.last
      end


      #
      # @return [Context] dump a {Context} object to <code>file_name</code>.
      #
      def dump_to file_name
        File.open( file_name, "w" ) do | f |
          f.print Marshal.dump( self )
        end
        self
      end


      #
      # @return [Context] load a {Context} object from <code>file_name</code>.
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
