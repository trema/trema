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

describe GetConfigReply, '.new( VALID OPTIONS )' do
  subject do
    GetConfigReply.new(
      :datapath_id => 123,
      :transaction_id => 1234,
      :flags => 1,
      :miss_send_len => 1024
    )
  end

  describe '#datapath_id' do
    subject { super().datapath_id }
    it { is_expected.to eq(123) }
  end

  describe '#transaction_id' do
    subject { super().transaction_id }
    it { is_expected.to eq(1234) }
  end

  describe '#flags' do
    subject { super().flags }
    it { is_expected.to eq(1) }
  end

  describe '#miss_send_len' do
    subject { super().miss_send_len }
    it { is_expected.to eq(1024) }
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
