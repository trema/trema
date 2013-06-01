#
# The controller class of phost.
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
require "trema/executables"


module Trema
  #
  # An interface class to phost emulation utility program.
  #
  class Phost
    include Trema::Daemon


    command { | phost | "sudo #{ Executables.phost } -i #{ phost.interface } -p #{ Trema.pid } -l #{ Trema.log } -n #{ phost.name } -D" }
    wait_until_up
    daemon_id :name


    #
    # Creates a new instance of Phost for each virtual host.
    #
    def initialize host
      @host = host
    end


    #
    # @raise [RuntimeError] if no interface defined for virtual host.
    #
    # @return [String] the virtual host's interface name.
    #
    def interface
      raise "The link(s) for vhost '#{ name }' is not defined." if @host.interface.nil?
      @host.interface
    end


    #
    # @return [Boolean] whether phost is running or not.
    #
    def running?
      not @host.interface.nil? and super
    end


    def name
      @host.name
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
