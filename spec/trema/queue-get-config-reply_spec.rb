#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
#
# Copyright (C) 2008-2011 NEC Corporation
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


describe QueueGetConfigReply do
  context "when an instance is created" do
    it "should have valid attributes" do
      for i in 1..2 do
        pq = PacketQueue.new( :queue_id => i, :len => i* 64 )
        mr = MinRateQueue.new( i, i * 64, 1024 * i, pq)
      end
      qr = QueueGetConfigReply.new( :datapath_id => 0xabc,
        :transaction_id => 123,
        :port => 1,
        :queues => Queue.queues
      )
      qr.datapath_id.should == 0xabc
      qr.transaction_id.should == 123
      qr.port.should == 1
      qr.queues[0].should be_an_instance_of( PacketQueue )
    end
  end
  
  
  context "when a PacketQueue instance is created" do
    it "should have valid attributes" do
      pq = PacketQueue.new( :queue_id => 123, :len => 64 )
      pq.queue_id.should == 123
      pq.len.should == 64
    end
  end
  
  
  context "when a MinRateQueue instance is created" do
    it "should have valid attributes"  do
      pq = PacketQueue.new( :queue_id => 123, :len => 64 )
      mr = MinRateQueue.new( 1, 64, 1024, pq )
      mr.property.should == 1
      mr.len.should == 64
      mr.rate.should == 1024
    end
  end
  
  
  context "when multiple PacketQueue instances created" do
    it "should support multiplicity of queues" do
      Queue.should have(3).queues
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
