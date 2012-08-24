#
# trema send_packets command.
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


require "trema/cli"
require "trema/dsl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def trema_send_packets command
      command.desc "host that sends packets"
      command.flag [ :s, :source ]

      command.desc "host that receives packets"
      command.flag [ :d, :dest ]

      command.default_value nil
      command.flag [ :inc_ip_src ]

      command.default_value nil
      command.flag [ :inc_ip_dst ]

      command.flag [ :tp_src ]
      command.default_value nil
      command.flag [ :inc_tp_src ]

      command.flag [ :tp_dst ]
      command.default_value nil
      command.flag [ :inc_tp_dst ]

      command.flag [ :pps ]
      command.flag [ :n_pkts ]
      command.flag [ :duration ]
      command.flag [ :length ]

      command.default_value nil
      command.flag [ :inc_payload ]

      command.action do | global_options, options, args |
        raise "--source option is a mandatory" if options[ :source ].nil?
        source = Trema::DSL::Context.load_current.hosts[ options[ :source ] ]
        raise "Unknown host: #{ options[ :source ] }" if source.nil?

        raise "--dest option is a mandatory" if options[ :dest ].nil?
        dest = Trema::DSL::Context.load_current.hosts[ options[ :dest ] ]
        raise "Unknown host: #{ options[ :dest ] }" if dest.nil?

        Trema::Cli.new( source ).send_packets( dest, options )
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
