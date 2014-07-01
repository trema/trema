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

describe QueueGetConfigReply, '.new(VALID OPTIONS)' do
  subject do
    for i in 1..2 do
      pq = PacketQueue.new(:queue_id => i, :len => i * 64)
      MinRateQueue.new(i, i * 64, 1024 * i, pq)
    end
    QueueGetConfigReply.new(
      :datapath_id => 0xabc,
      :transaction_id => 123,
      :port => 1,
      :queues => Trema::PacketQueues.queues
    )
  end

  describe '#queues' do
    subject { super().queues }
    describe '#first' do
      subject { super().first }
      it { is_expected.to be_an_instance_of PacketQueue }
    end
  end

  describe '#datapath_id' do
    subject { super().datapath_id }
    it { is_expected.to eq(0xabc) }
  end

  describe '#transaction_id' do
    subject { super().transaction_id }
    it { is_expected.to eq(123) }
  end
end

describe PacketQueue, '.new( VALID OPTIONS )' do
  subject { PacketQueue.new(:queue_id => 123, :len => 64) }

  describe '#queue_id' do
    subject { super().queue_id }
    it { is_expected.to eq(123) }
  end

  describe '#len' do
    subject { super().len }
    it { is_expected.to eq(64) }
  end
end

describe MinRateQueue, '.new( VALID OPTIONS )' do
  subject do
    pq = PacketQueue.new(:queue_id => 123, :len => 64)
    MinRateQueue.new(1, 64, 1024, pq)
  end

  describe '#property' do
    subject { super().property }
    it { is_expected.to eq(1) }
  end

  describe '#len' do
    subject { super().len }
    it { is_expected.to eq(64) }
  end

  describe '#rate' do
    subject { super().rate }
    it { is_expected.to eq(1024) }
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
