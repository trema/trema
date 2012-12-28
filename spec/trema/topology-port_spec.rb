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


include Trema::Topology

describe Trema::Topology, :nosudo => true do

  describe Port do
    it "should be initialized with Hash" do
      expect {
        port = Port.new( {:dpid => 0x1234, :portno => 42 } )
      }.not_to raise_error
    end

    it "should raise error if empty instance is being created" do
      expect {
        port = Port.new
      }.to raise_error()
    end

    it "should raise error if key :dpid missing" do
      expect {
        port = Port.new( {:data => 0x1234, :portno => 42 } )
      }.to raise_error(ArgumentError)
    end

    it "should raise error if key :portno missing" do
      expect {
        port = Port.new( {:dpid => 0x1234, :number => 42 } )
      }.to raise_error(ArgumentError)
    end

    it "should have dpid accessor" do
      port = Port.new( {:dpid => 0x1234, :portno => 42 } )
      expect( port.dpid ).to be == 0x1234
    end

    it "should have portno accessor" do
      port = Port.new( {:dpid => 0x1234, :portno => 42 } )
      expect( port.portno ).to be == 42
    end

    it "has method up?" do
      port = Port.new( {:dpid => 0x1234, :portno => 42, :up => true } )
      expect( port.up? ).to be_true

      port = Port.new( {:dpid => 0x1234, :portno => 42, :up => false } )
      expect( port.up? ).to be_false

      port = Port.new( {:dpid => 0x1234, :portno => 42 } )
      expect( port.up? ).to be_false
    end

    it "has method external?" do
      port = Port.new( {:dpid => 0x1234, :portno => 42, :external => true } )
      expect( port.external? ).to be_true

      port = Port.new( {:dpid => 0x1234, :portno => 42, :external => false } )
      expect( port.external? ).to be_false

      port = Port.new( {:dpid => 0x1234, :portno => 42 } )
      expect( port.external? ).to be_false
    end

    it "has method name" do
      port = Port.new( {:dpid => 0x1234, :portno => 42, :name => "Port name" } )
      expect( port.name ).to eq( "Port name" )
    end

    it "has method mac" do
      port = Port.new( {:dpid => 0x1234, :portno => 42, :mac => "08:00:27:00:34:B7" } )
      expect( port.mac ).to eq( "08:00:27:00:34:B7" )
    end

    it "should have method key" do
      port = Port.new( {:dpid => 0x1234, :portno => 42 } )
      expect( port.key ).to be == [0x1234,42]
    end

    it "should be serializable to human readable form by to_s" do
      port = Port.new( {:dpid => 0x1234, :portno => 42, :up => true, :not_used => 1 } )
      expect( port.to_s ).to be == "Port: 0x1234:42 - {not_used:1, up:true}"
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
