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


require "trema/app"
require "trema/host"
require "trema/link"
require "trema/packetin-filter"
require "trema/switch"
require "trema/switch-manager"


module Trema
  module DSL
    class Context
      def self.load_from file_name
        if FileTest.exists?( file_name )
          Marshal.load( IO.read file_name )
        else
          new
        end
      end
      
      
      def initialize
        @tremashark = nil
        @port = 6633

        @apps = Trema::App.instances.clear
        @hosts = Trema::Host.instances.clear
        @links = Trema::Link.instances.clear
        @packetin_filter = Trema::PacketinFilter.instances.clear
        @switch_manager = Trema::SwitchManager.instances.clear
        @switches = Trema::Switch.instances.clear
      end
      

      def dump_to file_name
        File.open( file_name, "w" ) do | f |
          f.print Marshal.dump( self )
        end
        self
      end
      

      ################################################################################
      # Read current context.
      ################################################################################

      attr_accessor :tremashark
      attr_accessor :port


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


      def links
        @links.values
      end
      
      
      def link_index
        @links.size
      end


      def apps
        @apps.values
      end


      def switch_manager
        if @switch_manager.values[ 0 ]
          @switch_manager.values[ 0 ]          
        elsif apps.size == 0
          rule = { :port_status => "default", :packet_in => "default", :state_notify => "default" }
          SwitchManager.new( rule, @port )
        elsif apps.size == 1
          app_name = apps[ 0 ].name
          rule = { :port_status => app_name, :packet_in => app_name, :state_notify => app_name }
          SwitchManager.new( rule, @port )
        else
          # two or more apps without switch_manager.
          raise "No event routing configured. Use `event' directive to specify event routing."
        end
      end


      def packetin_filter
        @packetin_filter.values[ 0 ]
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
