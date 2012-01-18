#
# trema send_packets command.
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


require "optparse"
require "trema/cli"
require "trema/dsl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def send_packets
      sanity_check

      source = nil
      dest = nil
      cli_options = {}

      dsl_parser = Trema::DSL::Parser.new

      options = OptionParser.new
      options.banner = "Usage: #{ $0 } send_packets --source HOSTNAME --dest HOSTNAME [OPTIONS ...]"

      options.on( "-s", "--source HOSTNAME" ) do | v |
        source = Trema::DSL::Context.load_current.hosts[ v ]
        raise "Unknown host: #{ v }" if source.nil?
      end
      options.on( "--inc_ip_src [NUMBER]" ) do | v |
        if v
          cli_options[ :inc_ip_src ] = v
        else
          cli_options[ :inc_ip_src ] = true
        end
      end
      options.on( "-d", "--dest HOSTNAME" ) do | v |
        dest = Trema::DSL::Context.load_current.hosts[ v ]
        raise "Unknown host: #{ v }" if dest.nil?
      end
      options.on( "--inc_ip_dst [NUMBER]" ) do | v |
        if v
          cli_options[ :inc_ip_dst ] = v
        else
          cli_options[ :inc_ip_dst ] = true
        end
      end
      options.on( "--tp_src NUMBER" ) do | v |
        cli_options[ :tp_src ] = v
      end
      options.on( "--inc_tp_src [NUMBER]" ) do | v |
        if v
          cli_options[ :inc_tp_src ] = v
        else
          cli_options[ :inc_tp_src ] = true
        end
      end
      options.on( "--tp_dst NUMBER" ) do | v |
        cli_options[ :tp_dst ] = v
      end
      options.on( "--inc_tp_dst [NUMBER]" ) do | v |
        if v
          cli_options[ :inc_tp_dst ] = v
        else
          cli_options[ :inc_tp_dst ] = true
        end
      end
      options.on( "--pps NUMBER" ) do | v |
        cli_options[ :pps ] = v
      end
      options.on( "--n_pkts NUMBER" ) do | v |
        cli_options[ :n_pkts ] = v
      end
      options.on( "--duration NUMBER" ) do | v |
        cli_options[ :duration ] = v
      end
      options.on( "--length NUMBER" ) do | v |
        cli_options[ :length ] = v
      end
      options.on( "--inc_payload [NUMBER]" ) do | v |
        if v
          cli_options[ :inc_payload ] = v
        else
          cli_options[ :inc_payload ] = true
        end
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

      Trema::Cli.new( source ).send_packets( dest, cli_options )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
