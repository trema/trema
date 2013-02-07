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
require "trema/link"


module Trema
  describe Link do
    before :each do
      Trema::Link.instances.clear
      @stanza = mock( "link stanza", :peers => [ "Virtual Host", "Virtual Switch" ] )
    end


    context "when creating/deleting a link" do
      before :each do
        @link = Link.new( @stanza )
      end


      it "executes ip and ifconfig command" do
        @link.should_receive( :sh ).once.ordered.with( "sudo ip link add name trema0-0 type veth peer name trema0-1" )
        @link.should_receive( :sh ).once.ordered.with( "sudo /sbin/sysctl -w net.ipv6.conf.trema0-0.disable_ipv6=1 >/dev/null 2>&1" )
        @link.should_receive( :sh ).once.ordered.with( "sudo /sbin/sysctl -w net.ipv6.conf.trema0-1.disable_ipv6=1 >/dev/null 2>&1" )
        @link.should_receive( :sh ).once.ordered.with( "sudo /sbin/ifconfig trema0-0 up" )
        @link.should_receive( :sh ).once.ordered.with( "sudo /sbin/ifconfig trema0-1 up" )

        @link.enable!
      end


      it "executes ip and ifconfig command" do
        @link.should_receive( :sh ).once.with( "sudo ip link delete trema0-0 2>/dev/null" )

        @link.delete!
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
