#
# Copyright (C) 2012 Hiroyasu OHYAMA
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


require "trema/hardware-switch"


module Trema
  class CustomSwitch < HardwareSwitch
    include Trema::Daemon


    log_file { |vswitch| "customswitch.#{ vswitch.name }.log" }

    
    def initialize stanza
      super stanza
    end


    def command
      "CHIBACH_TMP=#{ Trema.tmp } #{ path } -i #{ dpid_short } > #{ log_file } &"
    end


    ############################################################################
    private
    ############################################################################


    def path
      File.join( Trema.home, @stanza[ :path ] )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
