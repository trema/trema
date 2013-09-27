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


$LOAD_PATH << File.join( File.dirname( __FILE__ ), "..", "ruby" )
$LOAD_PATH.unshift File.expand_path( File.join File.dirname( __FILE__ ), "..", "vendor", "ruby-ifconfig-1.2", "lib" )


require "rubygems"

require "rspec"
require "rspec/autorun"
require "pio"
require "trema"
require "trema/dsl/configuration"
require "trema/dsl/context"
require "trema/ofctl"
require "trema/shell"
require "trema/util"


# Requires supporting files with custom matchers and macros, etc,
# in ./support/ and its subdirectories.
Dir[ "#{ File.dirname( __FILE__ ) }/support/**/*.rb" ].each do | each |
  require File.expand_path( each )
end


RSpec.configure do | config |
  config.expect_with :rspec do | c |
    # Ensure that 'expect' is used and disable 'should' for consistency
    c.syntax = :expect
  end
end


include Trema


def controller name
  Trema::App[ name ]
end


def vswitch name
  Trema::OpenflowSwitch[ name ]
end


def vhost name
  Trema::Host[ name ]
end


def send_packets source, dest, options = {}
  Trema::Shell.send_packets source, dest, options
end


include Trema::Util


class Network
  def initialize &block
    @context = Trema::DSL::Parser.new.eval( &block )
  end


  def run controller_class, &test
    begin
      trema_run controller_class
      test.call
    ensure
      trema_kill
    end
  end


  ################################################################################
  private
  ################################################################################


  def trema_run controller_class
    controller = controller_class.new
    if not controller.is_a?( Trema::Controller )
      raise "#{ controller_class } is not a subclass of Trema::Controller"
    end
    Trema::DSL::Context.new( @context ).dump

    app_name = controller.name
    rule = { :port_status => app_name, :packet_in => app_name, :state_notify => app_name }
    sm = SwitchManager.new( rule, @context.port )
    sm.no_flow_cleanup = true
    sm.run!

    @context.links.each do | name, each |
      each.add!
    end
    @context.hosts.each do | name, each |
      each.run!
    end
    @context.switches.each do | name, each |
      each.run!
      drop_packets_from_unknown_hosts each
    end
    @context.links.each do | name, each |
      each.up!
    end
    @context.hosts.each do | name, each |
      each.add_arp_entry @context.hosts.values - [ each ]
    end

    @th_controller = Thread.start do
      controller.run!
    end
    wait_until_controller_is_up app_name
  end


  def trema_kill
    cleanup_current_session
    @th_controller.join if @th_controller
    wait_until_all_pid_files_are_deleted
  end


  def drop_packets_from_unknown_hosts switch
    ofctl = Trema::Ofctl.new
    ofctl.add_flow switch, :priority => 0, :actions => "drop"
    @context.hosts.each do | name, each |
      ofctl.add_flow switch, :dl_type => "0x0800", :nw_src => each.ip, :priority => 1, :actions => "controller"
    end
  end


  def wait_until_controller_is_up trema_name, timeout = 10
    elapsed = 0
    loop do
      raise "Timed out waiting for #{ trema_name }." if elapsed > timeout
      break if FileTest.exists?( File.join( Trema.pid, "#{ trema_name }.pid" ) )
      sleep 1
      elapsed += 1
    end
    sleep 1
  end


  def wait_until_all_pid_files_are_deleted timeout = 10
    elapsed = 0
    loop do
      raise "Failed to clean up remaining processes." if elapsed > timeout
      break if Dir.glob( File.join( Trema.pid, "*.pid" ) ).empty?
      sleep 1
      elapsed += 1
    end
    sleep 1
  end
end


def network &block
  Network.new &block
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
