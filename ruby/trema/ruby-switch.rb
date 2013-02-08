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


require "rbconfig"
require "trema/hardware-switch"


module Trema
  class RubySwitch < HardwareSwitch
    include Trema::Daemon


    def initialize stanza
      super stanza
    end


    def command
      "#{ ruby } -I#{ libruby } -rtrema -e \"Trema.module_eval IO.read( '#{ @stanza.path }' )\" #{ dpid_short }"
    end


    ############################################################################
    private
    ############################################################################


    def libruby
      File.join Trema.home, "ruby"
    end


    def ruby
      File.join(
        RbConfig::CONFIG[ "bindir" ],
        RbConfig::CONFIG[ "ruby_install_name" ] + RbConfig::CONFIG[ "EXEEXT" ]
      )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
