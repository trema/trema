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


describe Error, ".new" do
  exception = "Type and code are mandatory arguments and should be specified."
  it "should raise '#{ exception }'" do
    expect { subject }.to raise_error( exception )
  end
end


describe Error, ".new( type, code )" do
  subject { Error.new( Error::OFPET_BAD_REQUEST, Error::OFPBRC_BAD_TYPE ) }
  its( :error_type ) { should == Error::OFPET_BAD_REQUEST }
  its( :code ) { should == Error::OFPBRC_BAD_TYPE }
  its( :user_data ) { should be_nil }
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe Error, ".new( type, code, transaction_id )" do
  subject { Error.new( Error::OFPET_BAD_ACTION, Error::OFPBAC_BAD_VENDOR, transaction_id ) }
  let( :transaction_id ) { 1234 }
  its( :error_type ) { should == Error::OFPET_BAD_ACTION }
  its( :code ) { should == Error::OFPBAC_BAD_VENDOR }
  its( :user_data ) { should be_nil }
  it_should_behave_like "any OpenFlow message"
end


describe Error, ".new( transaction_id, type, code, 'this is a test' )" do
  subject { Error.new( Error::OFPET_FLOW_MOD_FAILED, Error::OFPFMFC_BAD_EMERG_TIMEOUT, transaction_id, "this is a test" ) }
  let( :transaction_id ) { 1234 }
  its( :error_type ) { should == Error::OFPET_FLOW_MOD_FAILED }
  its( :code ) { should == Error::OFPFMFC_BAD_EMERG_TIMEOUT }
  its( :user_data ) { should eq "this is a test" }
  it_should_behave_like "any OpenFlow message"
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
