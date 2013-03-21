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

  describe Link do
    it "should be initialized with Hash" do
      expect {
        link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72  } )
      }.not_to raise_error
    end

    it "should raise error if empty instance is being created" do
      expect {
        link = Link.new
      }.to raise_error()
    end

    it "should raise error if key :from_dpid missing" do
      expect {
        link = Link.new( {:from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      }.to raise_error(ArgumentError)
    end

    it "should raise error if key :from_portno missing" do
      expect {
        link = Link.new( {:from_dpid => 0x1234, :to_dpid => 0x5678, :to_portno => 72 } )
      }.to raise_error(ArgumentError)
    end

    it "should raise error if key :to_dpid missing" do
      expect {
        link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_portno => 72 } )
      }.to raise_error(ArgumentError)
    end

    it "should raise error if key :to_portno missing" do
      expect {
        link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678 } )
      }.to raise_error(ArgumentError)
    end

    it "should have from_dpid accessor" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( link.from_dpid ).to be == 0x1234
    end

    it "should have from_dpid accessor" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( link.from_portno ).to be == 42
    end


    it "should have to_dpid accessor" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( link.to_dpid ).to be == 0x5678
    end

    it "should have to_dpid accessor" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( link.to_portno ).to be == 72
    end

    it "has method up?" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72, :up => true } )
      expect( link.up? ).to be_true

      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72, :up => false } )
      expect( link.up? ).to be_false

      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( link.up? ).to be_false
    end

    it "has method unstable?" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72, :unstable => true } )
      expect( link.unstable? ).to be_true

      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72, :unstable => false } )
      expect( link.unstable? ).to be_false

      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( link.unstable? ).to be_false
    end

    it "should have method key" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( link.key ).to be == [0x1234,42,0x5678,72]
    end

    it "should be serializable to human readable form by to_s" do
      link = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72, :up => true, :not_used => 1 } )
      expect( link.to_s ).to be == "Link: (0x1234:42)->(0x5678:72) - {not_used:1, up:true}"
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
