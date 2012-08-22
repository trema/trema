#
# trema kill command.
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


    def trema_kill command
      command.action do | global_options, options, args |
        context = Trema::DSL::Context.load_current

        # [FIXME] Trema apps does not appear in context.apps. why?
        pid_file = File.join( Trema.pid, "#{ args[ 0 ] }.pid" )
        if FileTest.exist?( pid_file )
          Trema::Process.read( pid_file ).kill!
          next
        end

        host = context.hosts[ args[ 0 ] ]
        if host
          host.shutdown
          next
        end

        switch = context.switches[ args[ 0 ] ]
        if switch
          switch.shutdown
          next
        end

        raise "Unknown name: #{ args[ 0 ] }"

        # [TODO] kill a link by its name. Needs a good naming convension for link.
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
