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


require File.join( File.dirname( __FILE__ ), "spec_helper" )
require "trema"


describe Trema do
  it { should have_constant :OFPC_FLOW_STATS }
  it { should have_constant :OFPC_TABLE_STATS }
  it { should have_constant :OFPC_PORT_STATS }
  it { should have_constant :OFPC_STP }
  it { should have_constant :OFPC_RESERVED }
  it { should have_constant :OFPC_IP_REASM }
  it { should have_constant :OFPC_QUEUE_STATS }
  it { should have_constant :OFPC_ARP_MATCH_IP }

  it { should have_constant :OFPAT_OUTPUT }
  it { should have_constant :OFPAT_SET_VLAN_VID }
  it { should have_constant :OFPAT_SET_VLAN_PCP }
  it { should have_constant :OFPAT_STRIP_VLAN }
  it { should have_constant :OFPAT_SET_DL_SRC }
  it { should have_constant :OFPAT_SET_DL_DST }
  it { should have_constant :OFPAT_SET_NW_SRC }
  it { should have_constant :OFPAT_SET_NW_DST }
  it { should have_constant :OFPAT_SET_NW_TOS }
  it { should have_constant :OFPAT_SET_TP_DST }
  it { should have_constant :OFPAT_ENQUEUE }
  it { should have_constant :OFPAT_VENDOR }

  it { should have_constant :OFPET_HELLO_FAILED }
  it { should have_constant :OFPHFC_INCOMPATIBLE }
  it { should have_constant :OFPHFC_EPERM }

  it { should have_constant :OFPET_BAD_REQUEST }
  it { should have_constant :OFPBRC_BAD_VERSION }
  it { should have_constant :OFPBRC_BAD_TYPE }
  it { should have_constant :OFPBRC_BAD_STAT }
  it { should have_constant :OFPBRC_BAD_VENDOR }
  it { should have_constant :OFPBRC_BAD_SUBTYPE }
  it { should have_constant :OFPBRC_EPERM }
  it { should have_constant :OFPBRC_BAD_LEN }
  it { should have_constant :OFPBRC_BUFFER_EMPTY }
  it { should have_constant :OFPBRC_BUFFER_UNKNOWN }

  it { should have_constant :OFPET_BAD_ACTION }
  it { should have_constant :OFPBAC_BAD_TYPE }
  it { should have_constant :OFPBAC_BAD_LEN }
  it { should have_constant :OFPBAC_BAD_VENDOR }
  it { should have_constant :OFPBAC_BAD_VENDOR_TYPE }
  it { should have_constant :OFPBAC_BAD_OUT_PORT }
  it { should have_constant :OFPBAC_BAD_ARGUMENT }
  it { should have_constant :OFPBAC_EPERM }
  it { should have_constant :OFPBAC_TOO_MANY }
  it { should have_constant :OFPBAC_BAD_QUEUE }

  it { should have_constant :OFPET_FLOW_MOD_FAILED }
  it { should have_constant :OFPFMFC_ALL_TABLES_FULL }
  it { should have_constant :OFPFMFC_OVERLAP }
  it { should have_constant :OFPFMFC_BAD_EMERG_TIMEOUT }
  it { should have_constant :OFPFMFC_BAD_COMMAND }
  it { should have_constant :OFPFMFC_UNSUPPORTED }

  it { should have_constant :OFPET_PORT_MOD_FAILED }
  it { should have_constant :OFPPMFC_BAD_PORT }
  it { should have_constant :OFPPMFC_BAD_HW_ADDR }

  it { should have_constant :OFPET_QUEUE_OP_FAILED }
  it { should have_constant :OFPQOFC_BAD_PORT }
  it { should have_constant :OFPQOFC_BAD_QUEUE }
  it { should have_constant :OFPQOFC_EPERM }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
