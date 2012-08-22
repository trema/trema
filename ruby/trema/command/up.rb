#
# trema up command.
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
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def trema_up command
      command.action do | global_options, options, args |
        context = Trema::DSL::Context.load_current

        switch = context.switches[ args[ 0 ] ]
        if switch
          switch.run
          next
        end

        # TODO: support vlink

        raise "Unknown name: #{ args[ 0 ] }"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
