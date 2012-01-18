#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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


describe FeaturesReply, ".new( VALID OPTIONS )" do
  subject { FeaturesReply.new( :datapath_id => 123, 
    :transaction_id => 1234,
    :n_buffers => 256,
    :n_tables => 2,
    :capabilities => 135,
    :actions => 2047,
    :ports => ports 
    )
  }
  its( :datapath_id ) { should == 123 }
  its( :transaction_id ) { should == 1234 }
  its( :n_buffers ) { should == 256 }
  its( :n_tables ) { should == 2 }
  its( :capabilities ) { should == 135 }
  its( :actions ) { should == 2047 }
  let( :ports ) { [ mock( "port #0" ), mock( "port #1" ), mock( "port #2" ) ] }
  its( :ports ) { subject.size.should == 3 }
end


describe FeaturesReply, ".new( MANDATORY OPTIONS MISSING)" do
  it "should raise ArgumentError" do
    expect { subject }.to raise_error( ArgumentError ) 
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
