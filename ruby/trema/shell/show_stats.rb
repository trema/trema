#
# show_stats command of Trema shell.
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


require "trema/dsl"


module Trema
  module Shell
    def show_stats host_name, option
      assert_trema_is_built

      raise "Host '#{ host_name }' is not defined." if Host[ host_name ].nil?
      raise "Host '#{ host_name }' is not connected to any link." if Host[ host_name ].interface.nil?

      if option.to_s == "tx"
        Cli.new( Host[ host_name ] ).show_tx_stats
      else
        Cli.new( Host[ host_name ] ).show_rx_stats
      end
      true
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
