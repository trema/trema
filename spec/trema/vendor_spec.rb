#
# Author: SUGYO Kazushi
#
# Copyright (C) 2012 NEC Corporation
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
require "trema"


describe Vendor, ".new( VALID OPTIONS )" do
  subject do
    Vendor.new(
      :datapath_id => 0xabc,
      :transaction_id => 1234,
      :vendor => 0x5555,
      :buffer => vendor_data
    )
  end
  its( :datapath_id ) { should == 0xabc }
  its( :transaction_id ) { should == 1234 }
  its( :vendor ) { should == 0x5555 }
  let( :vendor_data ) { [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 ] }
  its( :buffer ) { should == [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 ] }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
