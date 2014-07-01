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
require 'trema/executables'

describe Trema::Executables do
  subject { Trema::Executables }

  describe '#cli' do
    subject { super().cli }
    it { is_expected.to be_a(String) }
  end

  describe '#ovs_ofctl' do
    subject { super().ovs_ofctl }
    it { is_expected.to be_a(String) }
  end

  describe '#ovs_openflowd' do
    subject { super().ovs_openflowd }
    it { is_expected.to be_a(String) }
  end

  describe '#packet_capture' do
    subject { super().packet_capture }
    it { is_expected.to be_a(String) }
  end

  describe '#packetin_filter' do
    subject { super().packetin_filter }
    it { is_expected.to be_a(String) }
  end

  describe '#phost' do
    subject { super().phost }
    it { is_expected.to be_a(String) }
  end

  describe '#stdin_relay' do
    subject { super().stdin_relay }
    it { is_expected.to be_a(String) }
  end

  describe '#switch' do
    subject { super().switch }
    it { is_expected.to be_a(String) }
  end

  describe '#switch_manager' do
    subject { super().switch_manager }
    it { is_expected.to be_a(String) }
  end

  describe '#syslog_relay' do
    subject { super().syslog_relay }
    it { is_expected.to be_a(String) }
  end

  describe '#tremashark' do
    subject { super().tremashark }
    it { is_expected.to be_a(String) }
  end

  context 'when Trema is compiled' do
    before { allow(FileTest).to receive(:executable?).and_return(true) }

    describe '#compiled?' do
      subject { super().compiled? }
      it { is_expected.to be_truthy }
    end
  end

  context 'when Trema is not compiled' do
    before { allow(FileTest).to receive(:executable?).and_return(false) }

    describe '#compiled?' do
      subject { super().compiled? }
      it { is_expected.to be_falsey }
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
