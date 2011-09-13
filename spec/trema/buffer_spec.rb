#
# Author: Kazushi SUGYO
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
  describe Buffer do
    context "when an instance is created with no arguments" do
    subject { Buffer.new }
    its( :length ) { should == 0 }
    its( :size ) { should == 0 }
    its( :to_s ) { should == nil }
    its( :to_a ) { should == nil }
    end


    context "when an instance is created with argument set" do
      subject { Buffer.new( [ 1, 2, 3, 4 ].pack( "C*" ) ) }
      its( :length ) { should == 4 }
      its( :size ) { should == 4 }
      its( :to_s ) { should == "\001\002\003\004" }
      its( :to_a ) { should == [ 1, 2, 3, 4 ] }
    end


    context "when an instance is created with two arguments set" do
      subject { Buffer.new( [ 1, 2, 3, 4 ].pack( "C*" ), [ 5, 6, 7, 8, 9 ].pack( "C*" ) ) }
      its( :length ) { should == 9 }
      its( :size ) { should == 9 }
      its( :to_s ) { should == "\001\002\003\004\005\006\007\010\011" }
      its( :to_a ) { should == [ 1, 2, 3, 4, 5, 6, 7, 8, 9 ] }
    end

    context "when an instance is created with" do
      describe "number argment" do
        it "should raise an error" do
          expect {
            Buffer.new( 1234 )
          }.to raise_error( TypeError )
        end
      end


      describe "string and number argments" do
        it "should raise an error" do
          expect {
            Buffer.new( [ 1, 2, 3, 4 ].pack( "C*" ), 1234 )
          }.to raise_error( TypeError )
        end
      end


      describe "array argment" do
        it "should raise an error" do
          expect {
            Buffer.new( [ [ 1, 2, 3, 4 ].pack( "C*" ) ] )
          }.to raise_error( TypeError )
        end
      end


      describe "nil argment" do
        it "should raise an error" do
          expect {
            Buffer.new( nil )
          }.to raise_error( TypeError )
        end
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
