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


require "trema/network-component"
require "trema/process"


module Trema
  #
  # Trema applications
  #
  class App < NetworkComponent
    #
    # Creates a new Trema application from {DSL::App}
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
      self.class.add self
    end


    #
    # Returns the name of application
    #
    # @example
    #   app.name => "Trema Tetris"
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
    #   app.daemonize! => self
    #
    # @return [App]
    #
    # @api public
    #
    def daemonize!
      if options
        sh "#{ command } -d #{ options.join " " }"
      else
        sh "#{ command } -d"
      end
      self
    end


    #
    # Runs an application process
    #
    # @example
    #   app.run! => self
    #
    # @return [App]
    #
    # @api public
    #
    def run!
      if options
        sh "#{ command } #{ options.join " " }"
      else
        sh command
      end
      self
    end


    #
    # Kills running application
    #
    # @example
    #   app.shutdown!
    #
    # @return [undefined]
    #
    # @api public
    #
    def shutdown!
      Trema::Process.read( pid_file, @name ).kill!
    end


    ################################################################################
    private
    ################################################################################


    #
    # Returns the path of pid file
    #
    # @return [String]
    #
    # @api private
    #
    def pid_file
      File.join Trema.tmp, "#{ @name }.pid"
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


    #
    # Returns command line options
    #
    # @return [Array]
    #
    # @api private
    #
    def options
      @stanza[ :options ]
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
