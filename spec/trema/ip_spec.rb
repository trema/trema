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
require "trema/ip"


module Trema
  describe IP do
    context "when creating" do
      subject { IP.new( ip_address, prefixlen ) }

      context %{when "192.168.1.1/32"} do
        let( :ip_address ) { "192.168.1.1" }
        let( :prefixlen ) { 32 }

        its( :to_s ) { should == "192.168.1.1" }
        its( :to_i ) { should == 3232235777 }
        its( :to_a ) { should == [ 0xc0, 0xa8, 0x01, 0x01 ] }
      end

      context %{when "10.1.1.1/8"} do
        let( :ip_address ) { "10.1.1.1" }
        let( :prefixlen ) { 8 }

        its( :to_s ) { should == "10.0.0.0" }
        its( :to_i ) { should == 10 * 256 * 256 * 256 }
        its( :to_a ) { should == [ 0x0a, 0x00, 0x00, 0x00 ] }
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
