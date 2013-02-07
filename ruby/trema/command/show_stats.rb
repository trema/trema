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


require "trema/cli"
require "trema/util"


module Trema
  module Command
    include Trema::Util


    def trema_show_stats hostname, tx, rx
      cli = get_cli( hostname )

      unless tx || rx
        show_tx_and_rx cli
      end
      cli.show_tx_stats if tx
      cli.show_rx_stats if rx
    end


    ############################################################################
    private
    ############################################################################


    def show_tx_and_rx cli
      puts "Sent packets:"
      cli.show_tx_stats
      puts "Received packets:"
      cli.show_rx_stats
    end


    def get_cli name
      host = find_host_by_name( name )
      exit_now! "unknown host: #{ name }" if host.nil?
      Trema::Cli.new host
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
