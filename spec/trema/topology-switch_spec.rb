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

  describe Switch do
    describe "initialize" do
      it "should default to up state" do
        expect {
          s = Switch.new( {:dpid => 0x1234 } )
          expect( s.up? ).to be_true
        }.not_to raise_error
      end

      it "should raise error if empty instance is being created" do
        expect {
          s = Switch.new
        }.to raise_error()
      end

      it "should raise error if key :dpid missing" do
        expect {
          s = Switch.new( { :up => false } )
        }.to raise_error(ArgumentError)
      end
      
      it "should be down if Hash has :up => false" do
        s = Switch.new( { :dpid => 0x1234, :up => false } )
        expect( s.up? ).to be_false
      end
    end

    it "should have dpid accessor" do
      s = Switch.new( { :dpid => 0x1234, :magic => 42 } )
      expect( s.dpid ).to be == 0x1234
    end

    it "should have method up?" do
      s = Switch.new( { :dpid => 0x1234, :up => true } )
      expect( s.up? ).to be_true

      s = Switch.new( { :dpid => 0x1234, :up => false } )
      expect( s.up? ).to be_false

      s = Switch.new( { :dpid => 0x1234 } )
      expect( s.up? ).to be_true
    end

    it "should have method key" do
      s = Switch.new( { :dpid => 0x1234, :magic => 42 } )
      expect( s.key ).to be == 0x1234
    end

    it "should have port manipulation methods" do
      s = Switch.new( { :dpid => 0x1234, :magic => 42 } )

      s.add_port Port.new( { :dpid => 0x1234, :portno => 1 } )
      expect( s.ports[1] ).not_to be_nil

      s.delete_port Port.new( { :dpid => 0x1234, :portno => 2 } )
      expect( s.ports[2] ).to be_nil
    end

    it "should have link manipulation methods" do
      s1 = Switch.new( { :dpid => 0x1234, :magic => 42 } )
      s2 = Switch.new( { :dpid => 0x5678, :magic => 42 } )

      l1 = Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      l2 = Link.new( {:from_dpid => 0x5678, :from_portno => 72, :to_dpid => 0x1234, :to_portno => 42 } )

      s1.add_link l1
      expect( s1.links_out.empty? ).not_to be_true
      s2.add_link l1
      expect( s2.links_in.empty? ).not_to be_true

      s1.add_link l2
      expect( s1.links_in.empty? ).not_to be_true
      s2.add_link l2
      expect( s2.links_out.empty? ).not_to be_true

      s1.delete_link l2
      expect( s1.links_in.empty? ).to be_true
      #s1.delete_link l1
      s1.delete_link( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72 } )
      expect( s1.links_out.empty? ).to be_true

      s2.delete_link l1.key
      expect( s2.links_in.empty? ).to be_true
      s2.delete_link l2.key
      expect( s2.links_out.empty? ).to be_true
    end

    it "should be serializable to human readable form by to_s" do
      s = Switch.new( { :dpid => 0x1234, :up => true, :magic => 42 } )
      s.add_port Port.new( { :dpid => 0x1234, :portno => 42, :up => true, :not_used => 1 } )
      s.add_link Link.new( {:from_dpid => 0x1234, :from_portno => 42, :to_dpid => 0x5678, :to_portno => 72, :not_used => 1 } )
      s.add_link Link.new( {:from_dpid => 0xABCD, :from_portno => 102, :to_dpid => 0x1234, :to_portno => 42, :not_used => 1 } )
      expect( s.to_s ).to be == <<-EOS
Switch: 0x1234 - {magic:42, up:true}
 Port: 0x1234:42 - {not_used:1, up:true}
 Links_in
  <= 0xabcd:102
 Links_out
  => 0x5678:72
      EOS
    end
  end
end

### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
