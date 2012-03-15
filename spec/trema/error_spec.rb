#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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


describe Error, ".new( VALID OPTIONS )" do
  context "when using Error constants" do
    subject { Error.constants }
    it { should include :OFPET_HELLO_FAILED }
    it { should include :OFPET_BAD_REQUEST }
    it { should include :OFPET_BAD_ACTION }
    it { should include :OFPET_FLOW_MOD_FAILED }
    it { should include :OFPET_PORT_MOD_FAILED }
    it { should include :OFPET_QUEUE_OP_FAILED }

    it { should include :OFPHFC_INCOMPATIBLE }
    it { should include :OFPHFC_EPERM }

    it { should include :OFPBRC_BAD_VERSION }
    it { should include :OFPBRC_BAD_TYPE }
    it { should include :OFPBRC_BAD_STAT }
    it { should include :OFPBRC_BAD_VENDOR }
    it { should include :OFPBRC_BAD_SUBTYPE }
    it { should include :OFPBRC_EPERM }
    it { should include :OFPBRC_BAD_LEN }
    it { should include :OFPBRC_BUFFER_EMPTY }
    it { should include :OFPBRC_BUFFER_UNKNOWN }

    it { should include :OFPBAC_BAD_TYPE }
    it { should include :OFPBAC_BAD_LEN }
    it { should include :OFPBAC_BAD_VENDOR }
    it { should include :OFPBAC_BAD_VENDOR_TYPE }
    it { should include :OFPBAC_BAD_OUT_PORT }
    it { should include :OFPBAC_BAD_ARGUMENT }
    it { should include :OFPBAC_EPERM }
    it { should include :OFPBAC_TOO_MANY }
    it { should include :OFPBAC_BAD_QUEUE }

    it { should include :OFPFMFC_ALL_TABLES_FULL }
    it { should include :OFPFMFC_OVERLAP }
    it { should include :OFPFMFC_OVERLAP }
    it { should include :OFPFMFC_BAD_EMERG_TIMEOUT }
    it { should include :OFPFMFC_BAD_COMMAND }
    it { should include :OFPFMFC_UNSUPPORTED }

    it { should include :OFPPMFC_BAD_PORT }
    it { should include :OFPPMFC_BAD_HW_ADDR }

    it { should include :OFPQOFC_BAD_PORT }
    it { should include :OFPQOFC_BAD_QUEUE }
    it { should include :OFPQOFC_EPERM }
  end
end


describe Error, ".new( MANDATORY OPTIONS MISSING ) - type, code" do
  subject{ Error.new }
  exception = "Type and code are mandatory options"
  it "should raise '#{ exception }'" do
    expect { subject }.to raise_error( exception )
  end
end


describe Error, ".new( MANDATORY OPTION MISSING ) - code" do
  subject { Error.new( :type => Error::OFPET_BAD_REQUEST ) }
  exception = "Code is a mandatory option"
  it "should raise '#{ exception }'" do
    expect { subject }.to raise_error( exception )
  end
end


  
describe Error, ".new( MANDATORY OPTION MISSING ) - type" do
  subject { Error.new( :code => Error::OFPBRC_BAD_TYPE ) }
  exception = "Type is a mandatory option"
  it "should raise '#{ exception }'" do
    expect { subject }.to raise_error( exception )
  end
end


describe Error, ".new( OPTIONAL OPTION MISSING ) - transaction_id" do
  subject { Error.new( :type => Error::OFPET_BAD_REQUEST, :code => Error::OFPBRC_BAD_TYPE ) }
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe Error, ".new( OPTIONAL OPTION MISSING ) - user_data" do
    subject do 
      Error.new(
        :type => Error::OFPET_BAD_REQUEST,
        :code => Error::OFPBRC_BAD_TYPE,
        :transaction_id => 123
      )
    end
    its( :user_data ) { should be_nil }
  end


describe Error, ".new( VALID OPTIONS )" do
  subject do 
    Error.new(
      :type => Error::OFPET_BAD_REQUEST,
      :code => Error::OFPBRC_BAD_TYPE,
      :transaction_id => transaction_id,
      :user_data => "this is a test"
    )
  end
  let( :transaction_id ) { 1234 }
  its( :error_type ) { should == Error::OFPET_BAD_REQUEST }
  its( :code ) { should == Error::OFPBRC_BAD_TYPE }
  its( :user_data ) { should eq "this is a test" }
  it_should_behave_like "any OpenFlow message with transaction_id option"
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
