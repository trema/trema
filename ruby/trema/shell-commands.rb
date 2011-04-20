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


def run
  sanity_check

  begin
    cleanup_current_session
    if $run_as_daemon
      DSL::Runner.new( @config ).daemonize
    else
      DSL::Runner.new( @config ).run
    end
  ensure
    cleanup_current_session
  end
end


def drun
  sanity_check

  cleanup_current_session
  DSL::Runner.new( @config ).daemonize
end


def killall
  cleanup_current_session
end


def send_packets options
  sanity_check

  source = @config.hosts[ options[ :source ] ]
  dest = @config.hosts[ options[ :dest ] ]

  Cli.new.send_packets( source, dest, options )
end


def show_stats host_name, option
  sanity_check

  raise "Host '#{ host_name }' is not defined." if @config.hosts[ host_name ].nil?
  raise "Host '#{ host_name }' is not connected to any link." if @config.hosts[ host_name ].interface.nil?

  if option.to_s == "tx"
    Cli.new.show_tx_stats( @config.hosts[ host_name ] )
  else
    Cli.new.show_rx_stats( @config.hosts[ host_name ] )
  end
end


def reset_stats host_name
  sanity_check

  raise "Host '#{ host_name }' is not defined." if @config.hosts[ host_name ].nil?

  Cli.new.reset_stats( @config.hosts[ host_name ] )
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
