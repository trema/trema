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


describe Trema, ".constants" do
  subject { Trema.constants }

  it { should include "OFPC_FLOW_STATS" }
  it { should include "OFPC_TABLE_STATS" }
  it { should include "OFPC_PORT_STATS" }
  it { should include "OFPC_STP" }
  it { should include "OFPC_RESERVED" }
  it { should include "OFPC_IP_REASM" }
  it { should include "OFPC_QUEUE_STATS" }
  it { should include "OFPC_ARP_MATCH_IP" }

  it { should include "OFPAT_OUTPUT" }
  it { should include "OFPAT_SET_VLAN_VID" }
  it { should include "OFPAT_SET_VLAN_PCP" }
  it { should include "OFPAT_STRIP_VLAN" }
  it { should include "OFPAT_SET_DL_SRC" }
  it { should include "OFPAT_SET_DL_DST" }
  it { should include "OFPAT_SET_NW_SRC" }
  it { should include "OFPAT_SET_NW_DST" }
  it { should include "OFPAT_SET_NW_TOS" }
  it { should include "OFPAT_SET_TP_DST" }
  it { should include "OFPAT_ENQUEUE" }
  it { should include "OFPAT_VENDOR" }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
