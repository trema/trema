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

require 'rspec'

shared_examples_for 'any Openflow message with mandatory options' do | options |
  subject do
    opt_hash = {}
    options[ :options].each do | each |
      opt_hash[ each[ :name]] = __send__(each[ :name])
    end
    options[ :klass].new(opt_hash)
  end

  options[ :options].each do | each |
    let(each[ :name]) { each[ :sample_value] }
  end

  options[ :options].each do | each |
    context "with :#{ each[ :name] } (#{ each[ :sample_value] })" do
      let(each[ :name]) { each[ :sample_value] }

      describe each[ :name] do
        subject { super().send(each[ :name]) }
        it { is_expected.to eq(each[ :sample_value]) }
      end

      describe each[ :alias] do
        subject { super().send(each[ :alias]) }
        it { is_expected.to eq(each[ :sample_value]) }
      end if each[ :alias]
    end

    context "without :#{ each[ :name] }" do
      let(each[ :name]) { nil }
      it { expect { subject }.to raise_error(ArgumentError, ":#{ each[ :name] } is a mandatory option") }
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
