#
# trema show_stats command.
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


require "trema/cli"
require "trema/dsl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def trema_show_stats command
      command.desc "Show stats of packets sent"
      command.switch [ :t, :tx ]
      command.desc "Show stats of packets received"
      command.switch [ :r, :rx ]

      command.action do | global_options, options, args |
        sanity_check

        host = Trema::DSL::Context.load_current.hosts[ args[ 0 ] ]
        raise "Unknown host: #{ args[ 0 ] }" if host.nil?

        if options[ :tx ]
          Trema::Cli.new( host ).show_tx_stats
        elsif options[ :rx ]
          Trema::Cli.new( host ).show_rx_stats
        else
          puts "Sent packets:"
          Trema::Cli.new( host ).show_tx_stats
          puts "Received packets:"
          Trema::Cli.new( host ).show_rx_stats
        end
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
