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


module Trema
  describe Trema, ".constants", :nosudo => true do
    subject { Trema.constants }
    it { should include "OFPET_HELLO_FAILED" }
    it { should include "OFPHFC_INCOMPATIBLE" }
    it { should include "OFPHFC_EPERM" }

    it { should include "OFPET_BAD_REQUEST" }
    it { should include "OFPBRC_BAD_VERSION" }
    it { should include "OFPBRC_BAD_TYPE" }
    it { should include "OFPBRC_BAD_STAT" }
    it { should include "OFPBRC_BAD_VENDOR" }
    it { should include "OFPBRC_BAD_SUBTYPE" }
    it { should include "OFPBRC_EPERM" }
    it { should include "OFPBRC_BAD_LEN" }
    it { should include "OFPBRC_BUFFER_EMPTY" }
    it { should include "OFPBRC_BUFFER_UNKNOWN" }

    it { should include "OFPET_BAD_ACTION" }
    it { should include "OFPBAC_BAD_TYPE" }
    it { should include "OFPBAC_BAD_LEN" }
    it { should include "OFPBAC_BAD_VENDOR" }
    it { should include "OFPBAC_BAD_VENDOR_TYPE" }
    it { should include "OFPBAC_BAD_OUT_PORT" }
    it { should include "OFPBAC_BAD_ARGUMENT" }
    it { should include "OFPBAC_EPERM" }
    it { should include "OFPBAC_TOO_MANY" }
    it { should include "OFPBAC_BAD_QUEUE" }

    it { should include "OFPET_FLOW_MOD_FAILED" }
    it { should include "OFPFMFC_ALL_TABLES_FULL" }
    it { should include "OFPFMFC_OVERLAP" }
    it { should include "OFPFMFC_BAD_EMERG_TIMEOUT" }
    it { should include "OFPFMFC_BAD_COMMAND" }
    it { should include "OFPFMFC_UNSUPPORTED" }

    it { should include "OFPET_PORT_MOD_FAILED" }
    it { should include "OFPPMFC_BAD_PORT" }
    it { should include "OFPPMFC_BAD_HW_ADDR" }

    it { should include "OFPET_QUEUE_OP_FAILED" }
    it { should include "OFPQOFC_BAD_PORT" }
    it { should include "OFPQOFC_BAD_QUEUE" }
    it { should include "OFPQOFC_EPERM" }
  end


  describe Error, ".new", :nosudo => true do
    it { expect { subject }.to raise_error( ArgumentError, "Type and code are mandatory options" ) }
  end


  describe Error, ".new(nil)", :nosudo => true do
    it { expect { subject }.to raise_error( ArgumentError, "Type and code are mandatory options" ) }
  end


  describe Error, ".new(:type => value)", :nosudo => true do
    subject { Error.new( :type => OFPET_BAD_REQUEST ) }
    it { expect { subject }.to raise_error( ArgumentError, "Code is a mandatory option" ) }
  end


  describe Error, ".new(:code => value)", :nosudo => true do
    subject { Error.new( :code => OFPBRC_BAD_TYPE ) }
    it { expect { subject }.to raise_error( ArgumentError, "Type is a mandatory option" ) }
  end


  describe Error, ".new(:type => value, :code => value)" do
    subject { Error.new( :type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE ) }
    it_should_behave_like "any Openflow message with default transaction ID"
    its( :error_type ) { should == OFPET_BAD_REQUEST }
    its( :code ) { should == OFPBRC_BAD_TYPE }
    its( :data ) { should be_nil }
  end


  describe Error, ".new(:type => value, :code => value, :transaction_id => value)" do
    subject { Error.new( :type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE, :transaction_id => transaction_id ) }
    it_should_behave_like "any Openflow message with transaction ID"
  end


  describe Error, ".new(:type => value, :code => value, :xid => value)" do
    subject { Error.new( :type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE, :xid => xid ) }
    it_should_behave_like "any Openflow message with xid"
  end


  describe Error, ".new(:type => value, :code => value, :data => value)" do
    subject { Error.new( :type => OFPET_BAD_REQUEST, :code => OFPBRC_BAD_TYPE, :data => "deadbeef" ) }
    it_should_behave_like "any Openflow message with default transaction ID"
    its( :error_type ) { should == OFPET_BAD_REQUEST }
    its( :code ) { should == OFPBRC_BAD_TYPE }
    its( :data ) { should == "deadbeef" }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
