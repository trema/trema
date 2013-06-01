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


require "rubygems"
require "rspec"


shared_examples_for "option is within range" do | option, range |
  context "with #{ option } (#{ range })" do
    let( option ) { range.first }
    it { expect { subject }.not_to raise_error( ArgumentError ) }
  end


  context "with #{ option } (< #{ range.first })" do
    let( option ) { range.first - 1 }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end


  context "with #{ option } (> #{ range.last })" do
    let( option ) { range.last + 1 }
    it { expect { subject }.to raise_error( ArgumentError ) }
  end
end


RSpec.configure do | config |
  config.alias_it_should_behave_like_to :it_validates, "it validates"
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
