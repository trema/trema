#
# Class LLDP
#
# Author: Taishi Matsushita <clavedia@gmail.com>
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


module Trema


  class LLDP
    def probe datapath_id, port
      mac_over_12chars = datapath_id.to_s( 16 ).rjust( 12, "0" )
      length = mac_over_12chars.length
      src_mac = mac_over_12chars.slice( length - 12, 12 )
  
      src_mac.insert 2, ":"
      src_mac.insert 5, ":"
      src_mac.insert 8, ":"
      src_mac.insert 11, ":"
      src_mac.insert 14, ":"

      send_packet_out_lldp datapath_id, src_mac, port
    end
  end


end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
