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
require 'trema/host'

module Trema
  describe Host do
    before do
      Host.instances.clear

      @cli = double('cli')
      allow(Cli).to receive(:new).and_return(@cli)
    end

    describe :ip do
      context 'when ip is omitted' do
        before { @stanza = {} }

        subject { Host.new(@stanza).ip }

        it { is_expected.to eq('192.168.0.1') }
      end

      context "when ip \"192.168.100.100\"" do
        before { @stanza = { :ip => '192.168.100.100' } }

        subject { Host.new(@stanza).ip }

        it { is_expected.to eq('192.168.100.100') }
      end
    end

    describe :mac do
      context 'when mac is omitted' do
        before { @stanza = {} }

        subject { Host.new(@stanza).mac }

        it { is_expected.to eq('00:00:00:00:00:01') }
      end

      context "when mac \"00:00:00:aa:bb:cc\"" do
        before { @stanza = { :mac => '00:00:00:aa:bb:cc' } }

        subject { Host.new(@stanza).mac }

        it { is_expected.to eq('00:00:00:aa:bb:cc') }
      end
    end

    describe :netmask do
      context 'when netmask is omitted' do
        before { @stanza = {} }

        subject { Host.new(@stanza).netmask }

        it { is_expected.to eq('255.255.255.255') }
      end

      context "when netmask \"255.255.0.0\"" do
        before { @stanza = { :netmask => '255.255.0.0' } }

        subject { Host.new(@stanza).netmask }

        it { is_expected.to eq('255.255.0.0') }
      end
    end

    context 'when #add_arp_entries' do
      describe :cli do
        before do
          @host0 = Host.new(:name => 'HOST 0')
          @host1 = double('HOST 1')
          @host2 = double('HOST 2')
          @host3 = double('HOST 3')
        end

        it 'should add arp entries' do
          expect(@cli).to receive(:add_arp_entry).with(@host1)
          expect(@cli).to receive(:add_arp_entry).with(@host2)
          expect(@cli).to receive(:add_arp_entry).with(@host3)

          @host0.add_arp_entry [@host1, @host2, @host3]
        end
      end
    end

    context 'when #run!' do
      describe :cli do
        before do
          allow(Phost).to receive(:new).and_return(double('phost', :run! => nil))
        end

        it 'should set IP and MAC address' do
          expect(@cli).to receive(:set_ip_and_mac_address)

          Host.new(:name => "Yutaro's host").run!
        end

        context 'when promisc on' do
          before { allow(@cli).to receive(:set_ip_and_mac_address) }

          it 'should enable promisc' do
            expect(@cli).to receive(:enable_promisc)

            Host.new(:name => "Yutaro's host", :promisc => true).run!
          end
        end
      end
    end

    context 'when #send_packets' do
      describe :cli do
        before do
          @dest = double('dest')
          @options = double('options')
        end

        it 'should send_packets' do
          expect(@cli).to receive(:send_packets).with(@dest, @options)

          Host.new(:name => "Yutaro's host").send_packet @dest, @options
        end
      end
    end

    context 'when getting stats' do
      before do
        @stats = double('stats')
        @host = Host.new(:name => "Yutaro's host")
      end

      context 'when #tx_stats' do
        describe :cli do
          it 'should get tx stats' do
            expect(@cli).to receive(:tx_stats).and_return(@stats)

            expect(@host.tx_stats).to eq(@stats)
          end
        end
      end

      context 'when #rx_stats' do
        describe :cli do
          it 'should get rx stats' do
            expect(@cli).to receive(:rx_stats).and_return(@stats)

            expect(@host.rx_stats).to eq(@stats)
          end
        end
      end
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
