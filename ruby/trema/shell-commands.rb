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


def controller name
  @context.apps[ name ]
end


def switch name
  @context.switches[ name ]
end


def host name
  @context.hosts[ name ]
end
alias :vhost :host


def run
  sanity_check

  begin
    cleanup_current_session
    if $run_as_daemon
      DSL::Runner.new( @context ).daemonize
    else
      DSL::Runner.new( @context ).run
    end
  ensure
    cleanup_current_session
  end
end


def drun
  sanity_check

  DSL::Runner.new( @context ).daemonize
end


def killall
  cleanup_current_session
end


def send_packets source, dest, options = {}
  sanity_check

  Trema::Cli.new( host( source ) ).send_packets( host( dest ), options )
end


def show_stats host_name, option
  sanity_check

  raise "Host '#{ host_name }' is not defined." if Host[ host_name ].nil?
  raise "Host '#{ host_name }' is not connected to any link." if Host[ host_name ].interface.nil?

  if option.to_s == "tx"
    puts Trema::Cli.new( Host[ host_name ] ).tx_stats
  else
    puts Trema::Cli.new( Host[ host_name ] ).rx_stats
  end
end


def reset_stats host_name
  sanity_check

  raise "Host '#{ host_name }' is not defined." if Host[ host_name ].nil?

  Trema::Cli.new( Host[ host_name ] ).reset_stats
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
