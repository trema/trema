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
require "trema"


describe GetConfigReply, ".new( VALID OPTIONS )" do
  subject { GetConfigReply.new( :datapath_id => 123,
    :transaction_id => 1234,
    :flags => 1,
    :miss_send_len => 1024
    )
  }
  its ( :datapath_id ) { should == 123 }
  its ( :transaction_id ) { should == 1234 }
  its ( :flags ) { should == 1 }
  its ( :miss_send_len ) { should == 1024 }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
