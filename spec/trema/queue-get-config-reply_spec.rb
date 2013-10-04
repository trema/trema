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


describe QueueGetConfigReply, ".new( VALID OPTIONS )" do
  subject do
    for i in 1..2 do
      pq = PacketQueue.new( :queue_id => i, :len => i * 64 )
      mr = MinRateQueue.new( i, i * 64, 1024 * i, pq)
    end
    QueueGetConfigReply.new( :datapath_id => 0xabc,
      :transaction_id => 123,
      :port => 1,
      :queues => Trema::PacketQueues.queues
    )
  end
  its( "queues.length" ) { should ==  2  }
  its( "queues.first" ) { should be_an_instance_of PacketQueue }
  its( :datapath_id ) { should == 0xabc }
  its( :transaction_id ) { should == 123 }
end


describe PacketQueue, ".new( VALID OPTIONS )" do
  subject { PacketQueue.new( :queue_id => 123, :len => 64 ) }
  its( :queue_id ) { should == 123 }
  its( :len ) { should == 64 }
end


describe MinRateQueue, ".new( VALID OPTIONS )" do
  subject do
    pq = PacketQueue.new( :queue_id => 123, :len => 64 )
    MinRateQueue.new( 1, 64, 1024, pq )
  end
  its( :property ) { should == 1 }
  its( :len ) { should == 64 }
  its( :rate ) { should == 1024 }
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
