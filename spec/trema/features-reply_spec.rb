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
  describe FeaturesReply, ".new" do
    it { expect { subject }.to raise_error( ArgumentError ) }
  end


  describe FeaturesReply, ".new(options...)" do
    subject {
      valid_options = {
        :datapath_id => 123,
        :transaction_id => 1234,
        :n_buffers => 256,
        :n_tables => 2,
        :capabilities => 135,
        :actions => 2047,
        :ports => [ "port #0", "port #1", "port #2" ]
      }
      FeaturesReply.new( valid_options )
    }

    its( :datapath_id ) { should == 123 }
    its( :transaction_id ) { should == 1234 }
    its( :xid ) { should == 1234 }
    its( :n_buffers ) { should == 256 }
    its( :n_tables ) { should == 2 }
    its( :capabilities ) { should == 135 }
    its( :actions ) { should == 2047 }
    its( "ports.size" ) { should == 3 }
  end


  describe FeaturesReply, ".new(options...) (No :datapath_id)" do
    subject {
      FeaturesReply.new( {
        # :datapath_id => 123,
        :transaction_id => 1234,
        :n_buffers => 256,
        :n_tables => 2,
        :capabilities => 135,
        :actions => 2047,
        :ports => [ "port #0", "port #1", "port #2" ]
      } )
    }
    it { expect { subject }.to raise_error( ArgumentError, ":datapath_id is a mandatory option" ) }
  end


  describe FeaturesReply, ".new(options...) (No :transaction_id)" do
    subject {
      FeaturesReply.new( {
        :datapath_id => 123,
        # :transaction_id => 1234,
        :n_buffers => 256,
        :n_tables => 2,
        :capabilities => 135,
        :actions => 2047,
        :ports => [ "port #0", "port #1", "port #2" ]
      } )
    }
    it { expect { subject }.to raise_error( ArgumentError, ":transaction_id is a mandatory option" ) }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
