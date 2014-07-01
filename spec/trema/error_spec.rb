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

module Trema
  describe Error, '.new', :nosudo => true do
    it { expect { subject }.to raise_error(ArgumentError, 'Type and code are mandatory options') }
  end

  describe Error, '.new(nil)', :nosudo => true do
    it { expect { subject }.to raise_error(ArgumentError, 'Type and code are mandatory options') }
  end

  describe Error, '.new(:type => value)', :nosudo => true do
    subject { Error.new(:type => OFPET_BAD_REQUEST) }
    it { expect { subject }.to raise_error(ArgumentError, 'Code is a mandatory option') }
  end

  describe Error, '.new(:code => value)', :nosudo => true do
    subject { Error.new(:code => OFPBRC_BAD_TYPE) }
    it { expect { subject }.to raise_error(ArgumentError, 'Type is a mandatory option') }
  end

  describe Error, '.new(:type => value, :code => value)' do
    subject { Error.new(:type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE) }
    it_should_behave_like 'any Openflow message with default transaction ID'

    describe '#error_type' do
      subject { super().error_type }
      it { is_expected.to eq(OFPET_BAD_REQUEST) }
    end

    describe '#code' do
      subject { super().code }
      it { is_expected.to eq(OFPBRC_BAD_TYPE) }
    end

    describe '#data' do
      subject { super().data }
      it { is_expected.to be_nil }
    end
  end

  describe Error, '.new(:type => value, :code => value, :transaction_id => value)' do
    subject { Error.new(:type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE, :transaction_id => transaction_id) }
    it_should_behave_like 'any Openflow message with transaction ID'
  end

  describe Error, '.new(:type => value, :code => value, :xid => value)' do
    subject { Error.new(:type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE, :xid => xid) }
    it_should_behave_like 'any Openflow message with xid'
  end

  describe Error, '.new(:type => value, :code => value, :data => value)' do
    subject { Error.new(:type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE, :data => 'deadbeef') }
    it_should_behave_like 'any Openflow message with default transaction ID'

    describe '#error_type' do
      subject { super().error_type }
      it { is_expected.to eq(OFPET_BAD_REQUEST) }
    end

    describe '#code' do
      subject { super().code }
      it { is_expected.to eq(OFPBRC_BAD_TYPE) }
    end

    describe '#data' do
      subject { super().data }
      it { is_expected.to eq('deadbeef') }
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
