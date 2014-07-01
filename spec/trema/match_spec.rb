# encoding: utf-8
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

require File.join(File.dirname(__FILE__), '..', 'spec_helper')
require 'trema'

describe Match, '.new' do
  subject do
    Match.new(
      :in_port => 1,
      :dl_src => '00:00:00:00:00:01',
      :dl_dst => '00:00:00:00:00:02',
      :dl_vlan => 65_535,
      :dl_vlan_pcp => 0,
      :dl_type => 0x800,
      :nw_tos => 0,
      :nw_proto => 17,
      :nw_src => '192.168.0.1',
      :nw_dst => '192.168.0.0/24',
      :tp_src => 10,
      :tp_dst => 20
    )
  end

  describe '#in_port' do
    subject { super().in_port }
    it { is_expected.to eq(1) }
  end

  describe '#dl_src' do
    subject { super().dl_src }
    describe '#to_s' do
      subject { super().to_s }
      it { is_expected.to eq('00:00:00:00:00:01') }
    end
  end

  describe '#dl_dst' do
    subject { super().dl_dst }
    describe '#to_s' do
      subject { super().to_s }
      it { is_expected.to eq('00:00:00:00:00:02') }
    end
  end

  describe '#dl_vlan' do
    subject { super().dl_vlan }
    it { is_expected.to eq(65_535) }
  end

  describe '#dl_vlan_pcp' do
    subject { super().dl_vlan_pcp }
    it { is_expected.to eq(0) }
  end

  describe '#dl_type' do
    subject { super().dl_type }
    it { is_expected.to eq(0x800) }
  end

  describe '#nw_tos' do
    subject { super().nw_tos }
    it { is_expected.to eq(0) }
  end

  describe '#nw_proto' do
    subject { super().nw_proto }
    it { is_expected.to eq(17) }
  end

  describe '#nw_src' do
    subject { super().nw_src }
    describe '#to_s' do
      subject { super().to_s }
      it { is_expected.to eq('192.168.0.1') }
    end
  end

  describe '#nw_src' do
    subject { super().nw_src }
    describe '#prefixlen' do
      subject { super().prefixlen }
      it { is_expected.to eq(32) }
    end
  end

  describe '#nw_dst' do
    subject { super().nw_dst }
    describe '#to_s' do
      subject { super().to_s }
      it { is_expected.to eq('192.168.0.0') }
    end
  end

  describe '#nw_dst' do
    subject { super().nw_dst }
    describe '#prefixlen' do
      subject { super().prefixlen }
      it { is_expected.to eq(24) }
    end
  end

  describe '#tp_src' do
    subject { super().tp_src }
    it { is_expected.to eq(10) }
  end

  describe '#tp_dst' do
    subject { super().tp_dst }
    it { is_expected.to eq(20) }
  end

  describe '#to_s' do
    subject { super().to_s }
    it { is_expected.to eq('wildcards = 0x20000(nw_dst(8)), in_port = 1, dl_src = 00:00:00:00:00:01, dl_dst = 00:00:00:00:00:02, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 17, nw_src = 192.168.0.1/32, nw_dst = 192.168.0.0/24, tp_src = 10, tp_dst = 20') }
  end
end

describe Match, '.compare' do
  it 'Should match' do
    tester = Match.new(
      :in_port => 1,
      :dl_src => '00:00:00:00:00:01',
      :dl_dst => '00:00:00:00:00:02',
      :dl_vlan => 65_535,
      :dl_vlan_pcp => 0,
      :dl_type => 0x800,
      :nw_tos => 0,
      :nw_proto => 17,
      :nw_src => '192.168.0.1',
      :nw_dst => '192.168.0.2',
      :tp_src => 10,
      :tp_dst => 20
    )
    pattern = Match.new(
      :in_port => 1,
      :nw_src => '192.168.0.0/24',
      :nw_dst => '192.168.0.0/24'
    )
    expect(pattern.compare(tester)).to be_truthy
  end

  it 'Should not match' do
    tester = Match.new(
      :in_port => 1,
      :dl_src => '00:00:00:00:00:01',
      :dl_dst => '00:00:00:00:00:02',
      :dl_vlan => 65_535,
      :dl_vlan_pcp => 0,
      :dl_type => 0x800,
      :nw_tos => 0,
      :nw_proto => 17,
      :nw_src => '192.168.0.1',
      :nw_dst => '192.168.0.2',
      :tp_src => 10,
      :tp_dst => 20
    )
    pattern = Match.new(
      :in_port => 1,
      :nw_src => '10.0.0.0/8',
      :nw_dst => '10.0.0.0/8'
    )
    expect(pattern.compare(tester)).to be_falsey
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
