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

require File.join(File.dirname(__FILE__), '..', 'spec_helper')
require 'trema'

shared_examples_for 'any stats-request' do
  it_should_behave_like 'any Openflow message with default transaction ID'

  describe '#flags' do
    subject { super().flags }
    it { is_expected.to eq(0) }
  end
end

describe StatsRequest do
  context 'when .DescStatsRequest.new( VALID OPTIONS )' do
    subject { DescStatsRequest.new }
    it_should_behave_like 'any stats-request'
  end

  context 'when .FlowStatsRequest.new( MANDATORY OPTION MISSING )' do
    subject { FlowStatsRequest.new }
    it 'should raise ArgumentError' do
      expect { subject }.to raise_error(ArgumentError)
    end
  end

  context 'when .FlowStatsRequest.new( OPTIONAL OPTIONS MISSING )' do
    subject { FlowStatsRequest.new(:match => Match.new(:dl_type => 0x800, :nw_proto => 17)) }
    it_should_behave_like 'any stats-request'

    describe '#match' do
      subject { super().match }
      it { is_expected.to be_an_instance_of(Match) }
    end

    describe '#table_id' do
      subject { super().table_id }
      it { is_expected.to eq(0xff) }
    end

    describe '#out_port' do
      subject { super().out_port }
      it { is_expected.to eq(0xffff) }
    end
  end

  context 'when .FlowStatsRequest.new( VALID OPTIONS )' do
    subject do
      FlowStatsRequest.new(
        :match => Match.new(:dl_type => 0x800, :nw_proto => 17),
        :table_id => 1,
        :out_port => 2
      )
    end
    it_should_behave_like 'any stats-request'

    describe '#match' do
      subject { super().match }
      it { is_expected.to be_an_instance_of(Match) }
    end

    describe '#table_id' do
      subject { super().table_id }
      it { is_expected.to eq(1) }
    end

    describe '#out_port' do
      subject { super().out_port }
      it { is_expected.to eq(2) }
    end
  end

  context 'when .AggregateStatsRequest.new( MANDATORY OPTION MISSING )' do
    subject { AggregateStatsRequest.new }
    it 'should raise ArgumentError' do
      expect { subject }.to raise_error(ArgumentError)
    end
  end

  context 'when .AggregateStatsRequest.new( OPTIONAL OPTIONS MISSING )' do
    subject { AggregateStatsRequest.new(:match => Match.new(:dl_type => 0x800, :nw_proto => 17)) }
    it_should_behave_like 'any stats-request'

    describe '#match' do
      subject { super().match }
      it { is_expected.to be_an_instance_of(Match) }
    end

    describe '#table_id' do
      subject { super().table_id }
      it { is_expected.to eq(0xff) }
    end

    describe '#out_port' do
      subject { super().out_port }
      it { is_expected.to eq(0xffff) }
    end
  end

  context 'when .AggregateStatsRequest.new( VALID OPTIONS )' do
    subject do
      AggregateStatsRequest.new(
        :match => Match.new(:dl_type => 0x800, :nw_proto => 17),
        :table_id => 1,
        :out_port => 2
      )
    end
    it_should_behave_like 'any stats-request'

    describe '#match' do
      subject { super().match }
      it { is_expected.to be_an_instance_of(Match) }
    end

    describe '#table_id' do
      subject { super().table_id }
      it { is_expected.to eq(1) }
    end

    describe '#out_port' do
      subject { super().out_port }
      it { is_expected.to eq(2) }
    end
  end

  context 'when .TableStatsRequest.new( VALID OPTIONS )' do
    subject { TableStatsRequest.new }
    it_should_behave_like 'any stats-request'
  end

  context 'when .PortStasRequest.new( OPTIONAL OPTION MISSING )' do
    subject { PortStatsRequest.new }
    it_should_behave_like 'any stats-request'

    describe '#port_no' do
      subject { super().port_no }
      it { is_expected.to eq(0xffff) }
    end
  end

  context 'when .PortStasRequest.new( VALID OPTIONS )' do
    subject { PortStatsRequest.new :port_no => 1 }
    it_should_behave_like 'any stats-request'

    describe '#port_no' do
      subject { super().port_no }
      it { is_expected.to eq(1) }
    end
  end

  context 'when .QueueStatsRequest.new( OPTIONAL OPTIONS MISSING )' do
    subject { QueueStatsRequest.new }
    it_should_behave_like 'any stats-request'

    describe '#port_no' do
      subject { super().port_no }
      it { is_expected.to eq(0xfffc) }
    end

    describe '#queue_id' do
      subject { super().queue_id }
      it { is_expected.to eq(0xffffffff) }
    end
  end

  context 'when .QueueStatsRequest.new( VALID OPTIONS )' do
    subject { QueueStatsRequest.new :port_no => 1, :queue_id => 2 }
    it_should_behave_like 'any stats-request'

    describe '#port_no' do
      subject { super().port_no }
      it { is_expected.to eq(1) }
    end

    describe '#queue_id' do
      subject { super().queue_id }
      it { is_expected.to eq(2) }
    end
  end

  context 'when .VendorStatsRequest.new( OPTIONAL OPTION MISSING )' do
    subject { VendorStatsRequest.new }
    it_should_behave_like 'any stats-request'

    describe '#vendor_id' do
      subject { super().vendor_id }
      it { is_expected.to eq(0x00004cff) }
    end

    describe '#data' do
      subject { super().data }
      it { is_expected.to be_nil }
    end
  end

  context 'when .VendorStatsRequest.new( VALID OPTION )' do
    subject { VendorStatsRequest.new :vendor_id => 123 }
    it_should_behave_like 'any stats-request'

    describe '#vendor_id' do
      subject { super().vendor_id }
      it { is_expected.to eq(123) }
    end

    describe '#data' do
      subject { super().data }
      it { is_expected.to be_nil }
    end
  end

  context 'when .VendorStatsRequest.new(:data => value)' do
    subject { VendorStatsRequest.new :data => data }
    it_should_behave_like 'any stats-request'
    let(:data) { 'VENDOR DATA'.unpack('C*') }

    describe '#data' do
      subject { super().data }
      it { is_expected.to eq([86, 69, 78, 68, 79, 82, 32, 68, 65, 84, 65]) }
    end

    describe '#vendor_id' do
      subject { super().vendor_id }
      it { is_expected.to eq(0x00004cff) }
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
