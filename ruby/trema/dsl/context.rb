#
# The current context of DSL.
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


require "trema/host"
require "trema/switch"
require "trema/switch-manager"


module Trema
  module DSL
    class Context
      def initialize
        @tremashark = nil
        @port = 6633
        @switches = Trema::Switch.all
        @switches.clear
        @hosts = Trema::Host.all
        @hosts.clear
        @links = []
        @apps = []
        @packetin_filter = nil
        @switch_manager = nil
      end


      ################################################################################
      # Read current context.
      ################################################################################


      attr_reader :tremashark
      attr_reader :port
      attr_reader :apps
      attr_reader :links
      attr_reader :packetin_filter


      def hosts
        @hosts.values
      end


      def find_host name
        @hosts[ name ]
      end


      def switches
        @switches.values
      end


      def find_switch name
        @switches[ name ]
      end


      def link_index
        @links.size
      end


      def switch_manager
        if @switch_manager
          @switch_manager
        elsif @apps.size == 0
          rule = { :port_status => "default", :packet_in => "default", :state_notify => "default" }
          SwitchManager.new( rule, @port )
        elsif @apps.size == 1
          app_name = @apps.last.name
          rule = { :port_status => app_name, :packet_in => app_name, :state_notify => app_name }
          SwitchManager.new( rule, @port )
        else
          # two or more apps without switch_manager.
          raise "No event routing configured. Use `event' directive to specify event routing."
        end
      end


      ################################################################################
      # Update current context.
      ################################################################################


      attr_writer :tremashark
      attr_writer :port


      def add_link link
        peers = link.peers

        @hosts[ peers[ 0 ] ].interface = link.interfaces[ 0 ] if @hosts[ peers[ 0 ] ]
        @hosts[ peers[ 1 ] ].interface = link.interfaces[ 1 ] if @hosts[ peers[ 1 ] ]

        @switches[ peers[ 0 ] ].add_interface link.interfaces[ 0 ] if @switches[ peers[ 0 ] ]
        @switches[ peers[ 1 ] ].add_interface link.interfaces[ 1 ] if @switches[ peers[ 1 ] ]

        @links << link
      end


      def set_filter filter
        @packetin_filter = filter
      end


      def set_switch_manager switch_manager
        @switch_manager = switch_manager
      end


      def add_app app
        @apps << app
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
