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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema/packetin-filter"


module Trema
  describe PacketinFilter do
    it "should run packetin_filter with proper options" do
      packetin_filter = PacketinFilter.new( :lldp => "TopologyManager", :packet_in => "OpenFlowPingPong" )
      packetin_filter.should_receive( :sh ).once.with( /packetin_filter \-\-daemonize \-\-name=filter lldp::TopologyManager packet_in::OpenFlowPingPong$/ )

      packetin_filter.run!
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
