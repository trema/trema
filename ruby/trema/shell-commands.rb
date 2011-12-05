#
# Trema shell commands.
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


require "trema/dsl"


include Trema


def vswitch name = nil, &block
  stanza = Trema::DSL::Vswitch.new( name )
  stanza.instance_eval( &block )
  Trema::OpenVswitch.new( stanza, @context.port )
  true
end


def vhost name = nil, &block
  stanza = Trema::DSL::Vhost.new( name )
  stanza.instance_eval( &block ) if block
  Trema::Host.new( stanza )
  true
end


def link peer0, peer1
  stanza = Trema::DSL::Link.new( peer0, peer1 )
  link = Trema::Link.new( stanza )
  link.enable!

  if Trema::Switch[ peer0 ]
    Trema::Switch[ peer0 ].add_interface link.name
  end
  if Trema::Switch[ peer1 ]
    Trema::Switch[ peer1 ].add_interface link.name_peer
  end

  if Trema::Host[ peer0 ]
    Trema::Host[ peer0 ].interface = link.name
    Trema::Host[ peer0 ].run!
  end
  if Trema::Host[ peer1 ]
    Trema::Host[ peer1 ].interface = link.name_peer
    Trema::Host[ peer1 ].run!
  end

  true
end


def run controller
  sanity_check

  if controller
    controller = controller
    if /ELF/=~ `file #{ controller }`
      stanza = Trema::DSL::App.new
      stanza.path controller
      Trema::App.new stanza
    else
      require "trema"
      ARGV.replace controller.split
      $LOAD_PATH << File.dirname( controller )
      Trema.module_eval IO.read( controller )
    end
  end

  runner = DSL::Runner.new( @context )
  runner.maybe_run_switch_manager
  runner.maybe_run_switches

  @context.apps.values.last.daemonize!
end


def killall
  cleanup_current_session
end


def send_packets source, dest, options = {}
  sanity_check

  Trema::Cli.new( Trema::Host[ source ] ).send_packets( Trema::Host[ dest ], options )
end


def show_stats host_name, option
  sanity_check

  raise "Host '#{ host_name }' is not defined." if Trema::Host[ host_name ].nil?
  raise "Host '#{ host_name }' is not connected to any link." if Trema::Host[ host_name ].interface.nil?

  if option.to_s == "tx"
    Trema::Cli.new( Trema::Host[ host_name ] ).show_tx_stats
  else
    Trema::Cli.new( Trema::Host[ host_name ] ).show_rx_stats
  end
end


def reset_stats host_name
  sanity_check

  raise "Host '#{ host_name }' is not defined." if Trema::Host[ host_name ].nil?

  Trema::Cli.new( Trema::Host[ host_name ] ).reset_stats
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
