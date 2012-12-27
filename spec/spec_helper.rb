#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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
    sleep 2  # FIXME: wait until controller.up?
  end


  def trema_kill
    cleanup_current_session
    @th_controller.join if @th_controller
    sleep 2  # FIXME: wait until switch_manager.down?
  end


  def drop_packets_from_unknown_hosts switch
    ofctl = Trema::Ofctl.new
    ofctl.add_flow switch, :priority => 0, :actions => "drop"
    @context.hosts.each do | name, each |
      ofctl.add_flow switch, :dl_type => "0x0800", :nw_src => each.ip, :priority => 1, :actions => "controller"
    end
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
