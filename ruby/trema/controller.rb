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


require "trema/app"
require "trema/monkey-patch/integer"
require "trema/monkey-patch/string"
require "trema/timers"


module Trema
  #
  # The base class of Trema controller.
  #
  class Controller < App
    include Timers


    #
    # Callback invoked whenever a subclass of this class is created.
    # This adds the created object to the DB of controllers.
    #
    def self.inherited subclass
      subclass.new
    end


    def initialize
      App.add self
    end


    #
    # Run as a daemon.
    #
    def daemonize!
      fork do
        ::Process.setsid
        fork do
          STDIN.close
          STDOUT.reopen "/dev/null", "a"
          STDERR.reopen "/dev/null", "a"
          self.run!
        end
      end
    end


    #
    # Name of the controller.
    #
    # @return [String]
    #
    def name
      self.class.to_s.split( "::" ).last
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
