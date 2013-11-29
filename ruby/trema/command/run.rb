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


require "trema/default_openflow_channel_port"
require "trema/dsl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def trema_run options
      @config_file = options[ :conf ] || nil
      @openflow_port = options[ :port ] || DEFAULT_OPENFLOW_CHANNEL_PORT

      if options[ :daemonize ]
        $run_as_daemon = true
      end
      if options[ :tremashark ]
        $use_tremashark = true
      end
      if options[ :no_flow_cleanup ]
        $no_flow_cleanup = true
      end

      need_cleanup = ( not running? )

      if $run_as_daemon
        Trema::DSL::Runner.new( load_config ).daemonize
      else
        begin
          Trema::DSL::Runner.new( load_config ).run
        rescue SystemExit
          # This is OK
        ensure
          cleanup_current_session if need_cleanup
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
        config = Trema::DSL::Configuration.new
      end

      config.port = @openflow_port

      if ARGV[ 0 ]
        controller_file = ARGV[ 0 ].split.first
        if ruby_controller?
          require "trema"
          Object.__send__ :include, Trema
          ARGV.replace ARGV[ 0 ].split[ 1..-1 ]
          $LOAD_PATH << File.dirname( controller_file )
          load controller_file
        else
          # Assume that the controller is written in C
          stanza = Trema::DSL::Run.new
          stanza.path controller_file
          stanza.options ARGV[ 0 ].split[ 1..-1 ]
          Trema::App.new( stanza )
        end
      end

      config
    end


    def ruby_controller?
      /\.rb\Z/=~ ARGV[ 0 ].split.first
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
