# encoding: utf-8
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

require File.join(File.dirname(__FILE__), 'spec_helper')
require 'trema'

describe Trema do
  it { is_expected.to have_constant :OFPC_FLOW_STATS }
  it { is_expected.to have_constant :OFPC_TABLE_STATS }
  it { is_expected.to have_constant :OFPC_PORT_STATS }
  it { is_expected.to have_constant :OFPC_STP }
  it { is_expected.to have_constant :OFPC_RESERVED }
  it { is_expected.to have_constant :OFPC_IP_REASM }
  it { is_expected.to have_constant :OFPC_QUEUE_STATS }
  it { is_expected.to have_constant :OFPC_ARP_MATCH_IP }

  it { is_expected.to have_constant :OFPAT_OUTPUT }
  it { is_expected.to have_constant :OFPAT_SET_VLAN_VID }
  it { is_expected.to have_constant :OFPAT_SET_VLAN_PCP }
  it { is_expected.to have_constant :OFPAT_STRIP_VLAN }
  it { is_expected.to have_constant :OFPAT_SET_DL_SRC }
  it { is_expected.to have_constant :OFPAT_SET_DL_DST }
  it { is_expected.to have_constant :OFPAT_SET_NW_SRC }
  it { is_expected.to have_constant :OFPAT_SET_NW_DST }
  it { is_expected.to have_constant :OFPAT_SET_NW_TOS }
  it { is_expected.to have_constant :OFPAT_SET_TP_DST }
  it { is_expected.to have_constant :OFPAT_ENQUEUE }
  it { is_expected.to have_constant :OFPAT_VENDOR }

  it { is_expected.to have_constant :OFPET_HELLO_FAILED }
  it { is_expected.to have_constant :OFPHFC_INCOMPATIBLE }
  it { is_expected.to have_constant :OFPHFC_EPERM }

  it { is_expected.to have_constant :OFPET_BAD_REQUEST }
  it { is_expected.to have_constant :OFPBRC_BAD_VERSION }
  it { is_expected.to have_constant :OFPBRC_BAD_TYPE }
  it { is_expected.to have_constant :OFPBRC_BAD_STAT }
  it { is_expected.to have_constant :OFPBRC_BAD_VENDOR }
  it { is_expected.to have_constant :OFPBRC_BAD_SUBTYPE }
  it { is_expected.to have_constant :OFPBRC_EPERM }
  it { is_expected.to have_constant :OFPBRC_BAD_LEN }
  it { is_expected.to have_constant :OFPBRC_BUFFER_EMPTY }
  it { is_expected.to have_constant :OFPBRC_BUFFER_UNKNOWN }

  it { is_expected.to have_constant :OFPET_BAD_ACTION }
  it { is_expected.to have_constant :OFPBAC_BAD_TYPE }
  it { is_expected.to have_constant :OFPBAC_BAD_LEN }
  it { is_expected.to have_constant :OFPBAC_BAD_VENDOR }
  it { is_expected.to have_constant :OFPBAC_BAD_VENDOR_TYPE }
  it { is_expected.to have_constant :OFPBAC_BAD_OUT_PORT }
  it { is_expected.to have_constant :OFPBAC_BAD_ARGUMENT }
  it { is_expected.to have_constant :OFPBAC_EPERM }
  it { is_expected.to have_constant :OFPBAC_TOO_MANY }
  it { is_expected.to have_constant :OFPBAC_BAD_QUEUE }

  it { is_expected.to have_constant :OFPET_FLOW_MOD_FAILED }
  it { is_expected.to have_constant :OFPFMFC_ALL_TABLES_FULL }
  it { is_expected.to have_constant :OFPFMFC_OVERLAP }
  it { is_expected.to have_constant :OFPFMFC_BAD_EMERG_TIMEOUT }
  it { is_expected.to have_constant :OFPFMFC_BAD_COMMAND }
  it { is_expected.to have_constant :OFPFMFC_UNSUPPORTED }

  it { is_expected.to have_constant :OFPET_PORT_MOD_FAILED }
  it { is_expected.to have_constant :OFPPMFC_BAD_PORT }
  it { is_expected.to have_constant :OFPPMFC_BAD_HW_ADDR }

  it { is_expected.to have_constant :OFPET_QUEUE_OP_FAILED }
  it { is_expected.to have_constant :OFPQOFC_BAD_PORT }
  it { is_expected.to have_constant :OFPQOFC_BAD_QUEUE }
  it { is_expected.to have_constant :OFPQOFC_EPERM }
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
