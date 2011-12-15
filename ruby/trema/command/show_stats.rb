#
# trema show_stats command.
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


require "optparse"
require "trema/cli"
require "trema/dsl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def show_stats
      sanity_check

      stats = nil

      options = OptionParser.new
      options.banner = "Usage: #{ $0 } show_stats HOSTNAME [OPTIONS ...]"

      options.on( "-t", "--tx" ) do
        stats = :tx
      end
      options.on( "-r", "--rx" ) do
        stats = :rx
      end

      options.separator ""

      options.on( "-h", "--help" ) do
        puts options.to_s
        exit 0
      end
      options.on( "-v", "--verbose" ) do
        $verbose = true
      end

      options.parse! ARGV

      host = Trema::DSL::Parser.new.load_current.hosts[ ARGV[ 0 ] ]
      raise "Unknown host: #{ ARGV[ 0 ] }" if host.nil?

      case stats
      when :tx
        Trema::Cli.new( host ).show_tx_stats
      when :rx
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


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
