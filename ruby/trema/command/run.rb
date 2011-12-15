#
# trema run command.
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
require "trema/dsl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def run
      sanity_check

      options = OptionParser.new
      options.banner = "Usage: #{ $0 } run [OPTIONS ...]"

      options.on( "-c", "--conf FILE" ) do | v |
        @config_file = v
      end
      options.on( "-d", "--daemonize" ) do
        $run_as_daemon = true
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

      cleanup_current_session

      if $run_as_daemon
        Trema::DSL::Runner.new( load_config ).daemonize
      else
        begin
          Trema::DSL::Runner.new( load_config ).run
        ensure
          cleanup_current_session
        end
      end
    end


    ############################################################################
    private
    ############################################################################


    def load_config
      config = nil
      dsl_parser = Trema::DSL::Parser.new

      if @config_file
        config = dsl_parser.parse( @config_file )
      elsif FileTest.exists?( "./trema.conf" )
        config = dsl_parser.parse( "./trema.conf" )
      else
        config = Trema::DSL::Context.new
      end

      if ARGV[ 0 ]
        controller_file = ARGV[ 0 ].split.first
        if c_controller?
          stanza = Trema::DSL::App.new
          stanza.path controller_file
          stanza.options ARGV[ 0 ].split[ 1..-1 ]
          Trema::App.new( stanza )
        else
          # Ruby controller
          require "trema"
          ARGV.replace ARGV[ 0 ].split
          $LOAD_PATH << File.dirname( controller_file )
          Trema.module_eval IO.read( controller_file )
        end
      end

      config
    end


    def c_controller?
      /ELF/=~ `file #{ ARGV[ 0 ].split.first }`
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
