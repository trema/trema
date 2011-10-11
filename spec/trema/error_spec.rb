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


describe Error do
  before do
    @type = Error::OFPET_BAD_REQUEST
    @code = Error::OFPBRC_BAD_TYPE
    @user_data = "this is a test"
  end


  context "when .new" do
    exception = "Type and code are mandatory arguments and should be specified"
    it "should raise '#{ exception }'" do
      expect { subject }.to raise_error( exception )
    end
  end


  context "when .new(:type => type )" do
    subject { Error.new( :type => @type ) }
    exception = "Code is a mandatory option"
    it "should raise '#{ exception }'" do
      expect { subject }.to raise_error( exception )
    end
  end


  
  context "when .new(:code => code )" do
    subject { Error.new( :code => @code ) }
    exception = "Type is a mandatory option"
    it "should raise '#{ exception }'" do
      expect { subject }.to raise_error( exception )
    end
  end


  context "when .new( :type => type, :code => code )" do
    subject { Error.new( :type => @type, :code => @code ) }
    its( :error_type ) { should == @type }
    its( :code ) { should == @code }
    its( :user_data ) { should be_nil }
    it_should_behave_like "any Openflow message with default transaction ID"
  end


  context "when .new( :type => type, :code => code, :transaction_id => transaction_id )" do
    subject do 
      Error.new(
        :type => @type,
        :code => @code,
        :transaction_id => transaction_id
      )
    end
    let( :transaction_id ) { 1234 }
    its( :error_type ) { should == @type }
    its( :code ) { should == @code }
    its( :user_data ) { should be_nil }
    it_should_behave_like "any OpenFlow message"
  end


  context "when .new( Hash[ :type, type, :code, code, :transaction_id, transaction_id, :user_data, user_data ] )" do
    subject do 
      Error.new(
        :type => @type,
        :code => @code,
        :transaction_id => transaction_id,
        :user_data => @user_data
      )
    end
    let( :transaction_id ) { 1234 }
    its( :error_type ) { should == @type }
    its( :code ) { should == @code }
    its( :user_data ) { should eq @user_data }
    it_should_behave_like "any OpenFlow message"
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
