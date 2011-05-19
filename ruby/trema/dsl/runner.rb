#
# Runs DSL objects in right order.
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


module Trema
  module DSL
    class Runner
      def initialize config
        @config = config
      end


      def run
        maybe_run_trema_services
        maybe_run_apps
      end


      def daemonize
        maybe_run_trema_services
        maybe_daemonize_apps
      end


      ################################################################################
      private
      ################################################################################


      def maybe_run_trema_services
        maybe_run_tremashark
        maybe_run_switch_manager
        maybe_run_packetin_filter
        maybe_create_links
        maybe_run_hosts
        maybe_run_switches
      end


      def maybe_run_tremashark
        @config.tremashark.run if @config.tremashark
      end


      def maybe_run_switch_manager
        @config.switch_manager.run if @config.switch_manager
      end


      def maybe_run_packetin_filter
        @config.packetin_filter.run if @config.packetin_filter
      end


      def maybe_create_links
        maybe_delete_links # Fool proof
        @config.links.each do | each |
          each.up!
        end
      end


      def maybe_delete_links
        @config.links.each do | each |
          each.down!
        end
      end


      def maybe_run_hosts
        @config.hosts.each do | each |
          each.run
        end
      end


      def maybe_run_switches
        @config.switches.each do | each |
          each.run
        end

        @config.hosts.each do | each |
          each.add_arp_entry @config.hosts - [ each ]
        end
      end


      def maybe_run_apps
        return if @config.apps.empty?

        @config.apps[ 0..-2 ].each do | each |
          each.daemonize
        end
        trap( "SIGINT" ) do
          print( "\nterminated\n" )
          exit(0)
        end
        pid = ::Process.fork do
          @config.apps.last.run
        end
        ::Process.waitpid pid
      end


      def maybe_daemonize_apps
        @config.apps.each do | each |
          each.daemonize
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
