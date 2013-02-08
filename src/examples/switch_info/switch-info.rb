#
# Getting switch information
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


class SwitchInfo < Controller
  def switch_ready datapath_id
    send_message datapath_id, FeaturesRequest.new
  end


  def features_reply datapath_id, message
    info "datapath_id: %#x" % datapath_id
    info "transaction_id: %#x" % message.transaction_id
    info "n_buffers: %u" % message.n_buffers
    info "n_tables: %u" % message.n_tables
    info "capabilities: %u" % message.capabilities
    info "actions: %u" % message.actions
    info "#ports: %d" % message.ports.size
    shutdown!
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
