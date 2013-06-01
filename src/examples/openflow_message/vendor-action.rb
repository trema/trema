#
# "Vendor Action" sample application
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


class VendorActionSampleController < Controller
  NX_VENDOR_ID = 0x00002320
  NXAST_NOTE = 8

  def switch_ready datapath_id
    body = [ NXAST_NOTE, 0x54, 0x72, 0x65, 0x6d, 0x61, 0x00 ].pack( "nC6" )
    actions = VendorAction.new( NX_VENDOR_ID, body.unpack( "C*" ) )
    send_flow_mod_modify(
      datapath_id,
      :hard_timeout => 60,
      :match => Match.new,
      :actions => actions,
      :strict => true
    )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
