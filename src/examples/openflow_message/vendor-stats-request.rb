#
# "Vendor Stats Request" sample application
#
# Copyright (C) 2013 NEC Corporation
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


class VendorStatsRequestSample < Controller
  OVS_VENDOR_ID = 0x00002320
  OVSST_FLOW = 0

  def switch_ready datapath_id
    send_vendor_stats_request datapath_id
  end


  def stats_reply datapath_id, message
    if message.type == StatsReply::OFPST_VENDOR
      vendor_stats_reply datapath_id, message
    end
  end


  private


  def vendor_stats_reply datapath_id, message
    info "[vendor_stats_reply]"
    info "vendor_id: 0x%08x" % message.stats.first.vendor_id
    info "data: [#{ message.stats.first.data.map { | n |  "0x%02x" % n }.join ", " }]"
  end


  def send_vendor_stats_request datapath_id
    data = [
      OVSST_FLOW,
      0x0, 0x0, 0x0, 0x0,
      OFPP_NONE,
      0x0,
      0xff,
      0x0, 0x0, 0x0
    ].pack( "NC4 nnCC3" )
    vendor_request = VendorStatsRequest.new( :vendor_id => OVS_VENDOR_ID, :data => data.unpack( "C*" ) )
    send_message( datapath_id, vendor_request )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
