#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


describe Trema::FeaturesReply do
  it "should have datapath_id" do
    Trema::FeaturesReply.new( :datapath_id => 123 ).datapath_id.should == 123
  end


  it "should have transaction_id" do
    Trema::FeaturesReply.new( :transaction_id => 1234 ).transaction_id.should == 1234
  end


  it "should have n_buffers" do
    Trema::FeaturesReply.new( :n_buffers => 256 ).n_buffers.should == 256
  end


  it "should have n_tables" do
    Trema::FeaturesReply.new( :n_tables => 2 ).n_tables.should == 2
  end


  it "should have capabilities" do
    Trema::FeaturesReply.new( :capabilities => 135 ).capabilities.should == 135
  end


  it "should have actions" do
    Trema::FeaturesReply.new( :actions => 2047 ).actions.should == 2047
  end


  it "should have ports list" do
    ports = [ mock( "port #0" ), mock( "port #1" ), mock( "port #2" ) ]
    Trema::FeaturesReply.new( :ports => ports ).ports.size.should == 3
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:

