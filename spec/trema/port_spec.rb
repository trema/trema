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


describe Trema::Port do
  it "should have its port number" do
    port = Trema::Port.new( :number => 123 )
    expect( port.number ).to be( 123 )
  end

  it "should check the port up" do
    port = Trema::Port.new( :config => 0, :state => 0 )
    expect( port.up? ).to be_true
    expect( port.down? ).to be_false
  end

  it "should check the port down(config=0,state=1)" do
    port = Trema::Port.new( :config => 0, :state => 1 )
    expect( port.up? ).to be_false
    expect( port.down? ).to be_true
  end

  it "should check the port down(config=1,state=0)" do
    port = Trema::Port.new( :config => 1, :state => 0 )
    expect( port.up? ).to be_false
    expect( port.down? ).to be_true
  end

  it "should check the port down(config=1,state=1)" do
    port = Trema::Port.new( :config => 1, :state => 1 )
    expect( port.up? ).to be_false
    expect( port.down? ).to be_true
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
