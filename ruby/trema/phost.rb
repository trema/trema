#
# The controller class of phost.
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


require "trema/daemon"
require "trema/executables"


module Trema
  class Phost
    include Trema::Daemon


    daemon_id :interface


    def initialize host
      @host = host
    end


    def run
      sh "sudo #{ Executables.phost } -i #{ interface } -D"
      wait_until_up
    end


    ################################################################################
    private
    ################################################################################


    def interface
      raise "The link(s) for vhost '#{ name }' is not defined." if @host.interface.nil?
      @host.interface
    end


    def name
      @host.name
    end


    def wait_until_up
      loop do
        sleep 0.1
        break if FileTest.exists?( pid_file )
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
