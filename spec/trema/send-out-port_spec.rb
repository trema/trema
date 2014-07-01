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

describe SendOutPort, :type => 'actions' do
  describe '#new(port_number)' do
    subject(:send_out_port) { SendOutPort.new(port_number) }

    context 'with port_number (1)' do
      let(:port_number) { 1 }

      describe '#port_number' do
        subject { super().port_number }
        it { is_expected.to eq(1) }
      end

      describe '#max_len' do
        subject { super().max_len }
        it { is_expected.to eq(2**16 - 1) }
      end

      describe '#to_s' do
        subject { super().to_s }
        it { is_expected.to eq('Trema::SendOutPort: port_number=1, max_len=65535') }
      end
    end

    context 'with port_number (10)' do
      let(:port_number) { 10 }

      context "when set as FlowMod's action", :sudo => true do
        it 'should insert a new flow entry with action (output:10)' do
          class TestController < Controller; end
          network do
            vswitch { datapath_id 0xabc }
          end.run(TestController) do
            controller('TestController').send_flow_mod_add(0xabc, :actions => send_out_port)
            sleep 2
            expect(vswitch('0xabc').flows.size).to eq(1)
            expect(vswitch('0xabc').flows[ 0].actions).to eq('output:10')
            pending('Test actions as an object using Trema::Switch')
            expect(vswitch('0xabc').flows.size).to eq(1)
            expect(vswitch('0xabc').flows[ 0].actions.size).to eq(1)
            expect(vswitch('0xabc').flows[ 0].actions[ 0]).to be_a(SendOutPort)
            expect(vswitch('0xabc').flows[ 0].actions[ 0].port_number).to eq(10)
          end
        end
      end
    end

    it_validates 'option is within range', :port_number, 0..( 2**16 - 1)
  end

  describe '#new(:port_number => value)' do
    subject { SendOutPort.new(:port_number => port_number) }

    context 'with option (:port_number => 1)' do
      let(:port_number) { 1 }

      describe '#port_number' do
        subject { super().port_number }
        it { is_expected.to eq(1) }
      end

      describe '#max_len' do
        subject { super().max_len }
        it { is_expected.to eq(2**16 - 1) }
      end

      describe '#to_s' do
        subject { super().to_s }
        it { is_expected.to eq('Trema::SendOutPort: port_number=1, max_len=65535') }
      end
    end
  end

  describe '#new(:port_number => value1, :max_len => value2)' do
    subject { SendOutPort.new(options) }

    context 'with options (:port_number => 1, :max_len => 256)' do
      let(:options) { { :port_number => 1, :max_len => 256 } }

      describe '#port_number' do
        subject { super().port_number }
        it { is_expected.to eq(1) }
      end

      describe '#max_len' do
        subject { super().max_len }
        it { is_expected.to eq(256) }
      end

      describe '#to_s' do
        subject { super().to_s }
        it { is_expected.to eq('Trema::SendOutPort: port_number=1, max_len=256') }
      end
    end

    context 'with options (:port_number => 1, :max_len => max_len)' do
      let(:options) { { :port_number => 1, :max_len => max_len } }

      it_validates 'option is within range', :max_len, 0..( 2**16 - 1)
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
