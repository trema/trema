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


require "trema/util"


module Trema
  module Command
    include Trema::Util


    def trema_kill name
      unless maybe_kill( name )
        exit_now! "unknown name: #{ name }"
      end
    end


    ############################################################################
    private
    ############################################################################


    def maybe_kill name
      killed = maybe_kill_app( name ) || nil
      killed ||= maybe_shutdown_host( name )
      killed ||= maybe_shutdown_switch( name )
      # [TODO] kill a link by its name. Needs a good naming convension for link.
      killed
    end


    def maybe_kill_app name
      app = find_app_by_name( name )
      app.kill! if app
      app
    end


    def maybe_shutdown_host name
      host = find_host_by_name( name )
      host.shutdown! if host
      host
    end


    def maybe_shutdown_switch name
      switch = find_switch_by_name( name )
      switch.shutdown if switch
      switch
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
