#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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


require "trema/stats-helper"


module Trema
  #
  # PortStats Reply class
  # attributes mapped to ofp_port_stats
  #
  class PortStatsReply < StatsHelper
    FIELDS = %w(port_no rx_packets tx_packets rx_bytes) + 
      %w(tx_bytes rx_dropped tx_dropped rx_errors tx_errors) +
      %w(rx_frame_err rx_over_err rx_crc_err collisions )
    FIELDS.each { |field| attr_reader field.intern }


    def initialize options 
      super FIELDS, options
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
