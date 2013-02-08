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


class PacketInDumper < Controller
  def packet_in datapath_id, message
    info "received a packet_in"
    info "datapath_id: #{ datapath_id.to_hex }"
    info "transaction_id: #{ message.transaction_id.to_hex }"
    info "buffer_id: #{ message.buffer_id.to_hex }"
    info "total_len: #{ message.total_len }"
    info "in_port: #{ message.in_port }"
    info "reason: #{ message.reason.to_hex }"
    info "data: #{ message.data.unpack "H*" }"
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
