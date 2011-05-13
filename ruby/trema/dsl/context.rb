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
      attr_accessor :port
      attr_accessor :tremashark

      attr_reader :apps
      attr_reader :hosts
      attr_reader :links
      attr_reader :packetin_filter
      attr_reader :switch_manager
      attr_reader :switches
      
      
      def initialize
        @port = 6633
        @tremashark = nil
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
