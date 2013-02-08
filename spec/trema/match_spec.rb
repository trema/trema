#
# Copyright (C) 2008-2011 NEC Corporation
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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema"


describe Match, ".new" do
  subject {
    Match.new(
      :in_port => 1,
      :dl_src => "00:00:00:00:00:01",
      :dl_dst => "00:00:00:00:00:02",
      :dl_vlan => 65535,
      :dl_vlan_pcp => 0,
      :dl_type => 0x800,
      :nw_tos => 0,
      :nw_proto => 17,
      :nw_src => "192.168.0.1",
      :nw_dst => "192.168.0.0/24",
      :tp_src => 10,
      :tp_dst => 20
    )
  }
  its( :in_port ) { should == 1 }
  its( "dl_src.to_s" ) { should == "00:00:00:00:00:01" }
  its( "dl_dst.to_s" ) { should == "00:00:00:00:00:02" }
  its( :dl_vlan ) { should == 65535 }
  its( :dl_vlan_pcp ) { should == 0 }
  its( :dl_type ) { should == 0x800 }
  its( :nw_tos ) { should == 0 }
  its( :nw_proto ) { should == 17 }
  its( "nw_src.to_s" ) { should == "192.168.0.1" }
  its( "nw_src.prefixlen" ) { should == 32 }
  its( "nw_dst.to_s" ) { should == "192.168.0.0" }
  its( "nw_dst.prefixlen" ) { should == 24 }
  its( :tp_src ) { should == 10 }
  its( :tp_dst ) { should == 20 }
  its( :to_s ) { should == "wildcards = 0x20000(nw_dst(8)), in_port = 1, dl_src = 00:00:00:00:00:01, dl_dst = 00:00:00:00:00:02, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 17, nw_src = 192.168.0.1/32, nw_dst = 192.168.0.0/24, tp_src = 10, tp_dst = 20" }
end


describe Match, ".compare" do
  it "Should match" do
    tester = Match.new(
      :in_port => 1,
      :dl_src => "00:00:00:00:00:01",
      :dl_dst => "00:00:00:00:00:02",
      :dl_vlan => 65535,
      :dl_vlan_pcp => 0,
      :dl_type => 0x800,
      :nw_tos => 0,
      :nw_proto => 17,
      :nw_src => "192.168.0.1",
      :nw_dst => "192.168.0.2",
      :tp_src => 10,
      :tp_dst => 20
    )
    pattern = Match.new(
      :in_port => 1,
      :nw_src => "192.168.0.0/24",
      :nw_dst => "192.168.0.0/24"
    )
    expect( pattern.compare( tester ) ).to be_true
  end

  it "Should not match" do
    tester = Match.new(
      :in_port => 1,
      :dl_src => "00:00:00:00:00:01",
      :dl_dst => "00:00:00:00:00:02",
      :dl_vlan => 65535,
      :dl_vlan_pcp => 0,
      :dl_type => 0x800,
      :nw_tos => 0,
      :nw_proto => 17,
      :nw_src => "192.168.0.1",
      :nw_dst => "192.168.0.2",
      :tp_src => 10,
      :tp_dst => 20
    )
    pattern = Match.new(
      :in_port => 1,
      :nw_src => "10.0.0.0/8",
      :nw_dst => "10.0.0.0/8"
    )
    expect( pattern.compare( tester ) ).to be_false
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
