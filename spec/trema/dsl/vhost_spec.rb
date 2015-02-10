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

require File.join(File.dirname(__FILE__), '..', '..', 'spec_helper')
require 'trema/dsl/vhost'

module Trema
  module DSL
    describe Vhost do
      describe :name do
        context 'when name empty' do
          before { @vhost = Vhost.new }

          subject { @vhost[:name] }

          context 'and IP address empty' do
            it { is_expected.to be_nil }
          end

          context 'and IP address is 192.168.100.100' do
            before { @vhost.ip '192.168.100.100' }

            specify { is_expected.to eq('192.168.100.100') }
          end
        end

        context "when name is \"Yutaro's host\"" do
          before { @vhost = Vhost.new("Yutaro's host") }

          subject { @vhost[:name] }

          context 'and ip address empty' do
            it { is_expected.to eq("Yutaro's host") }
          end

          context 'and ip address 192.168.100.100' do
            before { @vhost.ip '192.168.100.100' }

            it { is_expected.to eq("Yutaro's host") }
          end
        end
      end

      describe :promisc do
        before { @vhost = Vhost.new("Yutaro's host") }

        subject { @vhost[:promisc] }

        context 'when promisc off' do
          before { @vhost.promisc 'off' }

          it { is_expected.to be_falsey }
        end

        context 'when promisc no' do
          before { @vhost.promisc 'no' }

          it { is_expected.to be_falsey }
        end

        context 'when promisc on' do
          before { @vhost.promisc 'on' }

          it { is_expected.to be_truthy }
        end

        context 'when promisc yes' do
          before { @vhost.promisc 'yes' }

          it { is_expected.to be_truthy }
        end

        context 'when promisc INVALID_VALUE' do
          specify do
            expect { @vhost.promisc 'INVALID_VALUE' }.to raise_error(Trema::DSL::SyntaxError)
          end
        end
      end

      describe :netmask do
        before { @vhost = Vhost.new("Yutaro's host") }

        subject { @vhost[:netmask] }

        context 'when netmask empty' do
          it { is_expected.to be_nil }
        end

        context 'when netmask is 255.255.255.0' do
          before { @vhost.netmask '255.255.255.0' }

          it { is_expected.to eq('255.255.255.0') }
        end
      end

      describe :mac do
        before { @vhost = Vhost.new("Yutaro's host") }

        subject { @vhost[:mac] }

        context 'when mac empty' do
          it { is_expected.to be_nil }
        end

        context 'when mac is 00:00:00:01:00:01' do
          before { @vhost.mac '00:00:00:01:00:01' }

          it { is_expected.to eq('00:00:00:01:00:01') }
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
