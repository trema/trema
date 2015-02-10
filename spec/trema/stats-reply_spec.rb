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

describe StatsReply, '.new( VALID OPTIONS )' do
  context 'when #desc-stats-reply is created' do
    subject do
      DescStatsReply.new(
        :mfr_desc => 'NEC Corporation',
        :hw_desc => 'no hardware description',
        :sw_desc => 'version xx.xx',
        :serial_num => '1234',
        :dp_desc => 'nec01'
      )
    end

    it { is_expected.to respond_to(:to_s) }

    describe '#mfr_desc' do
      subject { super().mfr_desc }
      it { is_expected.to eq('NEC Corporation') }
    end

    describe '#hw_desc' do
      subject { super().hw_desc }
      it { is_expected.to eq('no hardware description') }
    end

    describe '#sw_desc' do
      subject { super().sw_desc }
      it { is_expected.to eq('version xx.xx') }
    end

    describe '#serial_num' do
      subject { super().serial_num }
      it { is_expected.to eq('1234') }
    end

    describe '#dp_desc' do
      subject { super().dp_desc }
      it { is_expected.to eq('nec01') }
    end
  end

  context 'when #flow-stats-reply is created' do
    subject do
      actions = [ActionOutput.new(:port => 1)]
      match = Match.new
      FlowStatsReply.new(
        :length => 96,
        :table_id => 0,
        :match => match,
        :duration_sec => 3,
        :duration_nsec => 106_000_000,
        :priority => 65_535,
        :idle_timeout => 0,
        :hard_timeout => 0,
        :cookie => 866_942_928_268_820_481,
        :packet_count => 2,
        :byte_count => 128,
        :actions => actions
      )
    end

    it { is_expected.to respond_to(:to_s) }

    describe '#length' do
      subject { super().length }
      it { is_expected.to eq(96) }
    end

    describe '#table_id' do
      subject { super().table_id }
      it { is_expected.to eq(0) }
    end

    describe '#match' do
      subject { super().match }
      it { is_expected.to be_an_instance_of Match }
    end

    describe '#duration_sec' do
      subject { super().duration_sec }
      it { is_expected.to eq(3) }
    end

    describe '#duration_nsec' do
      subject { super().duration_nsec }
      it { is_expected.to eq(106_000_000) }
    end

    describe '#priority' do
      subject { super().priority }
      it { is_expected.to eq(65_535) }
    end

    describe '#idle_timeout' do
      subject { super().idle_timeout }
      it { is_expected.to eq(0) }
    end

    describe '#hard_timeout' do
      subject { super().hard_timeout }
      it { is_expected.to eq(0) }
    end

    describe '#cookie' do
      subject { super().cookie }
      it { is_expected.to eq(866_942_928_268_820_481) }
    end

    describe '#packet_count' do
      subject { super().packet_count }
      it { is_expected.to eq(2) }
    end

    describe '#byte_count' do
      subject { super().byte_count }
      it { is_expected.to eq(128) }
    end

    describe '#actions' do
      subject { super().actions }
      it { is_expected.not_to be_empty }
    end
  end

  context 'when aggregate-stats-reply is created' do
    subject do
      AggregateStatsReply.new(
        :packet_count => 2,
        :byte_count => 128,
        :flow_count =>  10
      )
    end

    it { is_expected.to respond_to(:to_s) }

    describe '#packet_count' do
      subject { super().packet_count }
      it { is_expected.to eq(2) }
    end

    describe '#byte_count' do
      subject { super().byte_count }
      it { is_expected.to eq(128) }
    end

    describe '#flow_count' do
      subject { super().flow_count }
      it { is_expected.to eq(10) }
    end
  end

  context 'when table-stats-reply is created' do
    subject do
      TableStatsReply.new(
        :table_id => 1,
        :name => 'classifier',
        :wildcards => 4_194_303,
        :max_entries => 1_048_576,
        :active_count => 4,
        :lookup_count => 4,
        :matched_count => 1
      )
    end

    it { is_expected.to respond_to(:to_s) }

    describe '#table_id' do
      subject { super().table_id }
      it { is_expected.to eq(1) }
    end

    describe '#name' do
      subject { super().name }
      it { is_expected.to eq('classifier') }
    end

    describe '#wildcards' do
      subject { super().wildcards }
      it { is_expected.to eq(4_194_303) }
    end

    describe '#max_entries' do
      subject { super().max_entries }
      it { is_expected.to eq(1_048_576) }
    end

    describe '#active_count' do
      subject { super().active_count }
      it { is_expected.to eq(4) }
    end

    describe '#lookup_count' do
      subject { super().lookup_count }
      it { is_expected.to eq(4) }
    end

    describe '#matched_count' do
      subject { super().matched_count }
      it { is_expected.to eq(1) }
    end
  end

  context 'when port-stats-reply is created' do
    subject do
      PortStatsReply.new(
        :port_no => 1,
        :rx_packets => 7,
        :tx_packets => 10,
        :rx_bytes => 1454,
        :tx_bytes => 2314,
        :rx_dropped => 1,
        :tx_dropped => 1,
        :rx_errors => 1,
        :tx_errors => 1,
        :rx_frame_err => 1,
        :rx_over_err => 1,
        :rx_crc_err => 1,
        :collisions => 1
      )
    end

    it { is_expected.to respond_to(:to_s) }

    describe '#port_no' do
      subject { super().port_no }
      it { is_expected.to eq(1) }
    end

    describe '#rx_packets' do
      subject { super().rx_packets }
      it { is_expected.to eq(7) }
    end

    describe '#tx_packets' do
      subject { super().tx_packets }
      it { is_expected.to eq(10) }
    end

    describe '#rx_bytes' do
      subject { super().rx_bytes }
      it { is_expected.to eq(1454) }
    end

    describe '#tx_bytes' do
      subject { super().tx_bytes }
      it { is_expected.to eq(2314) }
    end

    describe '#rx_dropped' do
      subject { super().rx_dropped }
      it { is_expected.to eq(1) }
    end

    describe '#tx_dropped' do
      subject { super().tx_dropped }
      it { is_expected.to eq(1) }
    end

    describe '#rx_errors' do
      subject { super().rx_errors }
      it { is_expected.to eq(1) }
    end

    describe '#tx_errors' do
      subject { super().tx_errors }
      it { is_expected.to eq(1) }
    end

    describe '#rx_frame_err' do
      subject { super().rx_frame_err }
      it { is_expected.to eq(1) }
    end

    describe '#rx_over_err' do
      subject { super().rx_over_err }
      it { is_expected.to eq(1) }
    end

    describe '#rx_crc_err' do
      subject { super().rx_crc_err }
      it { is_expected.to eq(1) }
    end

    describe '#collisions' do
      subject { super().collisions }
      it { is_expected.to eq(1) }
    end
  end

  context 'when queue-stats-reply is created' do
    subject do
      QueueStatsReply.new(
        :port_no => 1,
        :queue_id => 2,
        :tx_bytes => 1024,
        :tx_packets => 16,
        :tx_errors => 5
      )
    end

    it { is_expected.to respond_to(:to_s) }

    describe '#port_no' do
      subject { super().port_no }
      it { is_expected.to eq(1) }
    end

    describe '#queue_id' do
      subject { super().queue_id }
      it { is_expected.to eq(2) }
    end

    describe '#tx_bytes' do
      subject { super().tx_bytes }
      it { is_expected.to eq(1024) }
    end

    describe '#tx_packets' do
      subject { super().tx_packets }
      it { is_expected.to eq(16) }
    end

    describe '#tx_errors' do
      subject { super().tx_errors }
      it { is_expected.to eq(5) }
    end
  end

  context 'when vendor-stats-reply is created' do
    subject { VendorStatsReply.new(:vendor_id => 123) }

    it { is_expected.to respond_to(:to_s) }

    describe '#vendor_id' do
      subject { super().vendor_id }
      it { is_expected.to eq(123) }
    end

    describe '#data' do
      subject { super().data }
      it { is_expected.to be_nil }
    end
  end

  context 'when #stats_request(desc-stats) is sent' do
    it 'should #stats_reply(desc-stats)' do
      class DescStatsController < Controller; end
      network do
        vswitch('desc-stats') { datapath_id 0xabc }
      end.run(DescStatsController) do
        expect(controller('DescStatsController')).to receive(:stats_reply) do |datapath_id, message|
          expect(datapath_id).to eq(0xabc)
          expect(message.type).to eq(0)
          expect(message.stats[0].mfr_desc).to eq('Nicira Networks, Inc.')
          expect(message.stats[0].hw_desc).to eq('Open vSwitch')
          expect(message.stats[0]).to respond_to :to_s
        end

        controller('DescStatsController').send_message(
          0xabc,
          DescStatsRequest.new(:transaction_id => 1234)
        )
        sleep 2 # FIXME: wait to send_message
      end
    end
  end

  context 'when #stats_request(flow-stats) is sent' do
    it 'should #stats_reply(flow-stats)' do
      class FlowStatsController < Controller; end
      network do
        vswitch('flow-stats') { datapath_id 0xabc }
        vhost 'host1'
        vhost 'host2'
        link 'host1', 'flow-stats'
        link 'host2', 'flow-stats'
      end.run(FlowStatsController) do
        controller('FlowStatsController').send_flow_mod_add(
          0xabc,
          # match the UDP packet
          :match => Match.new(:dl_type => 0x800, :nw_proto => 17),
          # flood the packet
          :actions => ActionOutput.new(:port => FlowStatsController::OFPP_FLOOD)
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        # send two packets
        send_packets 'host1', 'host2', :n_pkts => 2
        sleep 2 # FIXME: wait to send_packets

        expect(controller('FlowStatsController')).to receive(:stats_reply) do |datapath_id, message|
          expect(datapath_id).to eq(0xabc)
          expect(message.type).to eq(1)
          expect(message.stats[0].packet_count).to eq(2)
          expect(message.stats[0]).to respond_to :to_s
        end
        match = Match.new(:dl_type => 0x800, :nw_proto => 17)
        controller('FlowStatsController').send_message(
          0xabc,
          FlowStatsRequest.new(:match => match)
        )
        sleep 2 # FIXME: wait to send_message
      end
    end
  end

  context 'when #stats_request(aggregate_stats) is sent' do
    it 'should #stats_reply(aggregate-stats) attributes' do
      class AggregateStatsController < Controller; end
      network do
        vswitch('aggregate-stats') { datapath_id 0xabc }
        vhost 'host1'
        vhost 'host2'
        link 'host1', 'aggregate-stats'
        link 'host2', 'aggregate-stats'
      end.run(AggregateStatsController) do
        controller('AggregateStatsController').send_flow_mod_add(
          0xabc,
          # match the UDP packet
          :match => Match.new(:dl_type => 0x800, :nw_proto => 17),
          # flood the packet
          :actions => ActionOutput.new(:port => AggregateStatsController::OFPP_FLOOD)
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        # send ten packets
        send_packets 'host1', 'host2', :n_pkts => 10
        sleep 2 # FIXME: wait to send_packets

        expect(controller('AggregateStatsController')).to receive(:stats_reply) do |datapath_id, message|
          expect(datapath_id).to eq(0xabc)
          expect(message.type).to eq(2)
          expect(message.stats[0].packet_count).to eq(10)
          expect(message.stats[0].flow_count).to eq(1)
          expect(message.stats[0]).to respond_to :to_s
        end
        match = Match.new(:dl_type => 0x800, :nw_proto => 17)
        controller('AggregateStatsController').send_message(
          0xabc,
          AggregateStatsRequest.new(:match => match, :table_id => 0xff)
        )
        sleep 2 # FIXME: wait to send_message
      end
    end
  end

  context 'when #stats_request(port-stats) is sent' do
    it 'should #stats_reply(port-stats)' do
      class PortStatsController < Controller; end
      network do
        vswitch('port-stats') { datapath_id 0xabc }
        vhost 'host1'
        vhost 'host2'
        link 'host1', 'port-stats'
        link 'host2', 'port-stats'
      end.run(PortStatsController) do
        controller('PortStatsController').send_flow_mod_add(
          0xabc,
          :match => Match.new(:dl_type => 0x800, :nw_proto => 17),
          :actions => ActionOutput.new(:port => PortStatsController::OFPP_FLOOD)
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        send_packets 'host1', 'host2'
        sleep 2 # FIXME: wait to send_packets

        expect(controller('PortStatsController')).to receive(:stats_reply) do |datapath_id, message|
          expect(datapath_id).to eq(0xabc)
          expect(message.type).to eq(4)
          expect(message.stats[0]).to be_an_instance_of(Trema::PortStatsReply)
          expect(message.stats[0]).to respond_to :to_s
        end
        controller('PortStatsController').send_message(
          0xabc,
          PortStatsRequest.new(:port_no => 1)
        )
        sleep 2 # FIXME: wait to send_message
      end
    end
  end

  context 'when #stats_request(table-stats) is sent' do
    it 'should #stats_reply(table-stats)' do
      class TableStatsController < Controller; end
      network do
        vswitch('table-stats') { datapath_id 0xabc }
        vhost 'host1'
        vhost 'host2'
        link 'host1', 'table-stats'
        link 'host2', 'table-stats'
      end.run(TableStatsController) do
        controller('TableStatsController').send_flow_mod_add(
          0xabc,
          :actions => ActionOutput.new(:port => TableStatsController::OFPP_FLOOD)
        )
        sleep 1 # FIXME: wait to send_flow_mod_add
        send_packets 'host1', 'host2'
        sleep 2 # FIXME: wait to send_packets

        expect(controller('TableStatsController')).to receive(:stats_reply) do |datapath_id, message|
          expect(datapath_id).to eq(0xabc)
          expect(message.type).to eq(3)
          expect(message.transaction_id).to eq(123)
          expect(message.stats[0]).to be_an_instance_of(Trema::TableStatsReply)
          expect(message.stats[0]).to respond_to :to_s
        end
        controller('TableStatsController').send_message(
          0xabc,
          TableStatsRequest.new(:transaction_id => 123)
        )
        sleep 2 # FIXME: wait to send_message
      end
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
