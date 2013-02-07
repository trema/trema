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


require "trema/daemon"
require "trema/network-component"


module Trema
  #
  # Trema applications
  #
  class App < NetworkComponent
    include Trema::Daemon


    #
    # @return [Trema::DSL::Stanza] a map of key-value pair settings
    #   for trema's dsl run{} syntax.
    #
    attr_reader :stanza


    command { | app | [ app.command, app.stanza[ :options ] ].compact.join " " }


    #
    # Creates a new Trema application from {Trema::DSL::Run}
    #
    # @example
    #   app = Trema::App.new( stanza )
    #
    # @return [App]
    #
    # @api public
    #
    def initialize stanza
      @stanza = stanza
      if /\.rb\Z/=~ @stanza.fetch( :name )  # ruby?
        require "trema"
        path = @stanza.fetch( :path )
        ARGV.replace [ path ]
        $LOAD_PATH << File.dirname( path )
        Trema.module_eval IO.read( path )
      else
        App.add self
      end
    end


    #
    # Returns the name of application
    #
    # @example
    #   app.name #=> "Trema Tetris"
    #
    # @return [String]
    #
    # @api public
    #
    def name
      @stanza[ :name ]
    end


    #
    # Runs as a daemon
    #
    # @example
    #   app.daemonize! #=> self
    #
    # @return [App]
    #
    # @api public
    #
    def daemonize!
      sh [ command, "-d", @stanza[ :options ] ].compact.join( " " )
      self
    end


    #
    # Returns application's command to execute
    #
    # @return [String]
    #
    # @api private
    #
    def command
      "#{ @stanza[ :path ] } --name #{ name }"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
