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


require "trema/default_openflow_channel_port"
require "trema/app"
require "trema/host"
require "trema/link"
require "trema/openflow-switch"
require "trema/packetin-filter"
require "trema/switch-manager"


module Trema
  module DSL
    #
    # The current configuration of Trema.
    #
    class Configuration
      #
      # set/get the port number for switch manager to listen to
      #
      # @example
      #   config.port = 5432
      #
      # @return [Number]
      #
      attr_accessor :port

      #
      # the hash of {App}
      #
      # @example
      #   p config.apps
      #   #=> {"trema tetris"=>#<Trema::App:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      attr_reader :apps

      #
      # the hash of {Host}
      #
      # @example
      #   p config.hosts
      #   #=> {"host #0"=>#<Trema::Host:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      attr_reader :hosts

      #
      # the hash of {Link}
      #
      # @example
      #   p config.links
      #   #=> {"link #0"=>#<Trema::Link:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      attr_reader :links

      #
      # the hash of {Switch}
      #
      # @example
      #   p config.switches
      #   #=> {"switch #0"=>#<Trema::OpenflowSwitch:0xb73c9328>, ...}
      #
      # @return [Hash]
      #
      attr_reader :switches


      attr_reader :netnss


      #
      # Creates a new Trema configuration
      #
      # @example
      #   config = Trema::DSL::Configuration.new
      #
      # @return [Configuration]
      #
      def initialize
        @port = DEFAULT_OPENFLOW_CHANNEL_PORT
        @apps = Trema::App.clear
        @hosts = Trema::Host.clear
        @links = Trema::Link.clear
        @packetin_filter = Trema::PacketinFilter.clear
        @switch_manager = Trema::SwitchManager.clear
        @switches = Trema::OpenflowSwitch.clear
        @netnss = Trema::Netns.clear
      end


      #
      # Returns {PacketinFilter} configuration
      #
      # @example
      #   config.packetin_filter => #<Trema::PacketinFilter:0xb73c9328>
      #
      # @return [PacketinFilter]
      #
      def packetin_filter
        @packetin_filter.values.last
      end


      #
      # Returns {SwitchManager} configuration
      #
      # @example
      #   config.switch_manager => #<Trema::SwitchManager:0xb73c9328>
      #
      # @return [SwitchManager]
      #
      def switch_manager
        @switch_manager.values.last
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
