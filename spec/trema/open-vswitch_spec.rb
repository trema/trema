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
require 'trema/open-vswitch'

module Trema
  describe OpenflowSwitch do
    around do | example |
      begin
        OpenflowSwitch.clear
        example.run
      ensure
        OpenflowSwitch.clear
      end
    end

    it 'should keep a list of vswitches' do
      OpenVswitch.new double('stanza 0', :name => 'vswitch 0', :validate => true)
      OpenVswitch.new double('stanza 1', :name => 'vswitch 1', :validate => true)
      OpenVswitch.new double('stanza 2', :name => 'vswitch 2', :validate => true)

      expect(OpenflowSwitch.size).to eq(3)
      expect(OpenflowSwitch[ 'vswitch 0']).not_to be_nil
      expect(OpenflowSwitch[ 'vswitch 1']).not_to be_nil
      expect(OpenflowSwitch[ 'vswitch 2']).not_to be_nil
    end
  end

  describe OpenVswitch, 'dpid = 0xabc' do
    subject do
      stanza = { :dpid_short => '0xabc', :dpid_long => '0000000000000abc', :ip => '127.0.0.1' }
      allow(stanza).to receive(:validate)
      allow(stanza).to receive(:name).and_return(name)
      OpenVswitch.new stanza
    end

    context 'when its name is not set' do
      let(:name) { '0xabc' }

      describe '#name' do
        subject { super().name }
        it { is_expected.to eq('0xabc') }
      end

      describe '#dpid_short' do
        subject { super().dpid_short }
        it { is_expected.to eq('0xabc') }
      end

      describe '#dpid_long' do
        subject { super().dpid_long }
        it { is_expected.to eq('0000000000000abc') }
      end

      describe '#network_device' do
        subject { super().network_device }
        it { is_expected.to eq('vsw_0xabc') }
      end
    end

    context 'when its name is set' do
      let(:name) { 'Otosan Switch' }

      describe '#name' do
        subject { super().name }
        it { is_expected.to eq('Otosan Switch') }
      end

      describe '#dpid_short' do
        subject { super().dpid_short }
        it { is_expected.to eq('0xabc') }
      end

      describe '#dpid_long' do
        subject { super().dpid_long }
        it { is_expected.to eq('0000000000000abc') }
      end

      describe '#network_device' do
        subject { super().network_device }
        it { is_expected.to eq('vsw_0xabc') }
      end
    end

    context 'when getting its flows' do
      let(:name) { '0xabc' }

      it 'should execute ofctl to get the flows' do
        ofctl = double('ofctl')
        allow(Ofctl).to receive(:new).and_return(ofctl)

        expect(ofctl).to receive(:users_flows).with(subject).once

        subject.flows
      end
    end

    context 'when running it' do
      let(:name) { '0xabc' }

      it 'should execute ovs openflowd' do
        pending
        expect(subject).to receive(:sh).with { |command|
          expect(command).to include(Executables.ovs_openflowd)
        }

        subject.run!
      end

      it 'should be connected to virtual ports' do
        pending
        subject << 'VirtualInterface0'
        subject << 'VirtualInterface1'
        subject << 'VirtualInterface2'

        expect(subject).to receive(:sh).with { |command|
          expect(command).to include('--ports=VirtualInterface0,VirtualInterface1,VirtualInterface2')
        }

        subject.run!
      end
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
