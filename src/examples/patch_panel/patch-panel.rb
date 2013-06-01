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


class PatchPanel < Controller
  def start
    @patch = []
    File.open( "./patch-panel.conf" ).each_line do | each |
      if /^(\d+)\s+(\d+)$/=~ each
        @patch << [ $1.to_i, $2.to_i ]
      end
    end
  end


  def switch_ready datapath_id
    @patch.each do | port_a, port_b |
      make_patch datapath_id, port_a, port_b
    end
  end


  private


  def make_patch datapath_id, port_a, port_b
    send_flow_mod_add(
      datapath_id,
      :match => Match.new( :in_port => port_a ),
      :actions => SendOutPort.new( port_b )
    )
    send_flow_mod_add(
      datapath_id,
      :match => Match.new( :in_port => port_b ),
      :actions => SendOutPort.new( port_a )
    )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
