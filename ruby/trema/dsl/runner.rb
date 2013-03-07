#
# Runs DSL objects in right order.
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


require "trema/tremashark"


module Trema
  module DSL
    class Runner
      def initialize context
        @context = context
      end


      def run
        maybe_run_trema_services
        maybe_run_apps
      end


      def daemonize
        maybe_run_trema_services
        maybe_daemonize_apps
      end


      def maybe_run_switch_manager
        # FIXME
        return if FileTest.exists? File.join( Trema.pid, "switch_manager.pid" )

        switch_manager =
          if @context.switch_manager and @context.apps.values.size > 0
            last_app = @context.apps.values.last.name
            if not @context.switch_manager.rule.has_key?( :port_status )
              @context.switch_manager.rule[ :port_status ] = last_app
            end
            if not @context.switch_manager.rule.has_key?( :packet_in )
              @context.switch_manager.rule[ :packet_in ] = last_app
            end
            if not @context.switch_manager.rule.has_key?( :state_notify )
              @context.switch_manager.rule[ :state_notify ] = last_app
            end
            if not @context.switch_manager.rule.has_key?( :vendor )
              @context.switch_manager.rule[ :vendor ] = last_app
            end
            @context.switch_manager
          else
            if @context.apps.values.size == 0
              rule = { :port_status => "default", :packet_in => "default", :state_notify => "default", :vendor => "default" }
            elsif @context.apps.values.size == 1
              app_name = @context.apps.values[ 0 ].name
              rule = { :port_status => app_name, :packet_in => app_name, :state_notify => app_name, :vendor => app_name }
            else
              # two or more apps without switch_manager.
              raise "No event routing configured. Use `event' directive to specify event routing."
            end
            SwitchManager.new( rule, @context.port )
          end
        switch_manager.run!
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
        maybe_run_netnss
      end


      def maybe_run_tremashark
        Trema::Tremashark.new.run if $use_tremashark
      end


      def maybe_run_packetin_filter
        @context.packetin_filter.run! if @context.packetin_filter
      end


      def maybe_create_links
        maybe_delete_links # Fool proof
        @context.links.each do | name, link |
          link.enable!
        end
      end


      def maybe_delete_links
        @context.links.each do | name, link |
          link.delete!
        end
      end


      def maybe_run_hosts
        @context.hosts.each do | name, host |
          host.run!
        end
      end


      def maybe_run_switches
        @context.switches.each do | name, switch |
          switch.run!
        end

        @context.hosts.each do | name, host |
          host.add_arp_entry @context.hosts.values - [ host ]
        end
      end


      def maybe_run_netnss
        @context.netnss.each do | name, netns |
          netns.run!
        end
      end


      def maybe_run_apps
        return if @context.apps.values.empty?

        @context.apps.values[ 0..-2 ].each do | each |
          each.daemonize!
        end
        trap( "SIGINT" ) do
          print( "\nterminated\n" )
          exit(0)
        end
        pid = ::Process.fork do
          @context.apps.values.last.run!
        end
        ::Process.waitpid pid
      end


      def maybe_daemonize_apps
        @context.apps.each do | name, app |
          app.daemonize!
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
