#
# Dumps packet-in message.
#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


class PacketinDumper < Controller
  def packet_in datapath_id, event
    puts "received a packet_in"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ event.transaction_id.to_hex }"
    info "buffer_id: #{ event.buffer_id.to_hex }"
    info "total_len: #{ event.total_len }"
    info "in_port: #{ event.in_port }"
    info "reason: #{ event.reason.to_hex }"
    info "data: #{ event.data.unpack "H*" }"
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
