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


module Trema
  describe Error do
    context "when an instance is created with no arguments" do  
      before( :all ) do
        @error = Error.new
      end


      it "should have a valid transaction ID" do
        @error.transaction_id.should >= 0
      end


      it "should have a valid error type" do
        @error.error_type.should == Error::OFPET_HELLO_FAILED
      end


      it "should have a valid error code" do
        @error.code.should == Error::ERROR_CODES[ Error::OFPET_HELLO_FAILED ].first
      end


      it "should have no user data payload" do
        @error.user_data.should be_nil
      end
    end


    context "when an instance is created with two arguments(type,code)" do
      before( :all ) do
        @error = Error.new(
          Error::OFPET_BAD_REQUEST, 
          Error::ERROR_CODES[ Error::OFPET_BAD_REQUEST ][ 1 ]
        )
      end


      it "should have a valid transaction ID" do
        @error.transaction_id.should >= 0
      end


      it "should have the specified error type" do
        @error.error_type.should == Error::OFPET_BAD_REQUEST
      end


      it "should have the specified error code" do
        @error.code.should == Error::ERROR_CODES[ Error::OFPET_BAD_REQUEST ][ 1 ]
      end


      it "should have no user data payload" do
        @error.user_data.should be_nil
      end
    end


    context "when an instance is created with three arguments(transaction ID,type,code)" do
      before( :all ) do
        @error = Error.new(
          1234,
          Error::OFPET_BAD_ACTION,
          Error::ERROR_CODES[ Error::OFPET_BAD_ACTION ][ 2 ]
        )
      end


      it "should have the specified transaction ID" do
        @error.transaction_id.should == 1234
      end


      it "should have the specified error type" do
        @error.error_type.should == Error::OFPET_BAD_ACTION
      end


      it "should have the specified error code" do
        @error.code.should == Error::ERROR_CODES[ Error::OFPET_BAD_ACTION ][ 2 ]
      end


      it "should have no user data payload" do
        @error.user_data.should be_nil
      end
    end


    context "when an instance is created with all four arguments" do
      before( :all ) do
        @error = Error.new(
          6789,
          Error::OFPET_FLOW_MOD_FAILED,
          Error::ERROR_CODES[ Error::OFPET_FLOW_MOD_FAILED ][ 3 ],
          "this is a test"
        )
      end


      it "should have the specified transaction ID" do
        @error.transaction_id.should == 6789
      end


      it "should have the specified error type" do
        @error.error_type.should == Error::OFPET_FLOW_MOD_FAILED
      end


      it "should have the specified error code" do
        @error.code.should == Error::ERROR_CODES[ Error::OFPET_FLOW_MOD_FAILED ][ 3 ]
      end


      it "should have the specified user data payload" do
        @error.user_data.should eq( "this is a test" )
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
