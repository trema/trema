#
# The controller class of ovs-ofctl.
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


require "trema/executables"
require "trema/flow"


module Trema
  class Ofctl
    def add_flow switch, options
      actions = options[ :actions ]
      options.delete :actions
      option_string = options.collect do | k, v |
        "#{ k }=#{ v }"
      end.join( "," )
      sh "sudo #{ Executables.ovs_ofctl } add-flow #{ switch.network_device } #{ option_string },actions=#{ actions } 2>/dev/null"
    end


    def flows switch
      dump_flows( switch ).split( "\n" )[ 1..-1 ].collect do | each |
        Trema::Flow.parse( each )
      end.compact
    end


    def users_flows switch
      flows( switch ).select( &:users_flow? )
    end


    def dump_flows switch
      `sudo #{ Executables.ovs_ofctl } dump-flows #{ switch.network_device } 2>&1`
    end


    def bring_port_up switch, port_number
      `sudo #{ Executables.ovs_ofctl } mod-port #{ switch.network_device } #{ port_number } up 2>&1`
    end


    def bring_port_down switch, port_number
      `sudo #{ Executables.ovs_ofctl } mod-port #{ switch.network_device } #{ port_number } down 2>&1`
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
