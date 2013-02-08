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


shared_examples_for "port status message" do | options |
  it_should_behave_like(
    "any Openflow message with mandatory options",
    :klass => options[ :klass ],
    :options => [ { :name => :datapath_id, :alias => :dpid, :sample_value => 0xabc },
                  { :name => :transaction_id, :alias => :xid, :sample_value => 123 },
                  { :name => :phy_port, :sample_value => "PHY_PORT" } ]
  )
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
