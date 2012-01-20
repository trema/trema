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


describe FeaturesRequest, ".new( OPTIONAL OPTION MISSING )" do
  it_should_behave_like "any Openflow message with default transaction ID"
end


describe FeaturesRequest, ".new( VALID OPTION )" do
  subject { FeaturesRequest.new :transaction_id => transaction_id }
  it_should_behave_like "any OpenFlow message with transaction_id option"


  context "when #features_request is sent with transaction ID(1234)" do
    it "should receive #features_reply with transaction ID(1234)" do
      class FeaturesController < Controller; end
      network {
        vswitch { datapath_id 0xabc }
      }.run( FeaturesController ) {
        features_request = FeaturesRequest.new( :transaction_id => 1234 )
        controller( "FeaturesController" ).send_message( 0xabc, features_request )
        controller( "FeaturesController" ).should_receive( :features_reply ) do | arg |
          arg.datapath_id.should == 0xabc
          arg.transaction_id.should == 1234
        end
      }
    end
  end
end


describe FeaturesRequest, ".new( INVALID OPTION )" do
  it "should raise TypeError" do
    expect {
      FeaturesRequest.new "INVALID_OPTION"
    }.to raise_error( TypeError )
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
