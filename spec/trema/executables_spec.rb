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
require "trema/executables"


describe Trema::Executables do
  subject { Trema::Executables }

  its ( :cli ) { should be_a( String ) }
  its ( :ovs_ofctl ) { should be_a( String ) }
  its ( :ovs_openflowd ) { should be_a( String ) }
  its ( :packet_capture ) { should be_a( String ) }
  its ( :packetin_filter ) { should be_a( String ) }
  its ( :phost ) { should be_a( String ) }
  its ( :stdin_relay ) { should be_a( String ) }
  its ( :switch ) { should be_a( String ) }
  its ( :switch_manager ) { should be_a( String ) }
  its ( :syslog_relay ) { should be_a( String ) }
  its ( :tremashark ) { should be_a( String ) }

  context "when Trema is compiled" do
    before { FileTest.stub!( :executable? ).and_return( true ) }
    its ( :compiled? ) { should be_true }
  end

  context "when Trema is not compiled" do
    before { FileTest.stub!( :executable? ).and_return( false ) }
    its ( :compiled? ) { should be_false }
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
