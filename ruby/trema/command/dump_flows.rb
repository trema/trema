#
# trema dump_flows command.
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


require "trema/dsl"
require "trema/ofctl"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def trema_dump_flows command
      command.action do | global_options, options, args |
        sanity_check

        args.each do | each |
          switch = Trema::DSL::Context.load_current.switches[ each ]
          if switch.nil?
            raise "No switch named `#{ each }' found!"
          end
          puts Trema::Ofctl.new.dump_flows( switch )
        end
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
