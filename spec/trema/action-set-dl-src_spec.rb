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


describe Trema::ActionSetDlSrc do
  context "when an instance is created" do
    subject { Trema::ActionSetDlSrc.new( Mac.new( "52:54:00:a8:ad:8c" ) ) }
    its ( :dl_src ) { should be_an_instance_of( Trema::Mac ) }
    it { should respond_to( :to_s ) }
    it "should print its attributes" do
      subject.to_s.should == "#<Trema::ActionSetDlSrc> dl_src = 52:54:00:a8:ad:8c"
    end
    
    
    it "should append its action to a list of actions" do
      openflow_actions = double( )
      subject.should_receive( :append ).with( openflow_actions )
      subject.append( openflow_actions )
    end
    
    
    context "when dl_src is not supplied" do
      it "should raise an error" do
        lambda do
          Trema::ActionSetDlSrc.new( )
        end.should raise_error( ArgumentError )
      end
    end
  

    context "when dl_src is not a Trema::Mac object" do  
      it "should raise an error" do
        lambda do
          Trema::ActionSetDlSrc.new( 1234 )
        end.should raise_error( ArgumentError, /dl src address should be a Mac object/ )
      end
    end
  end
end

  ### Local variables:
  ### mode: Ruby
  ### coding: utf-8-unix
  ### indent-tabs-mode: nil
  ### End:
