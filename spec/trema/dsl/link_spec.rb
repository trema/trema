#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
#
# Copyright (C) 2008-2012 NEC Corporation
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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )
require "trema/dsl/link"


module Trema
  module DSL
    describe Link do
      context %[when parsing "link peerA peerB"] do
        it %[recognizes "link peerA peerB" directive] do
          lambda do
            Link.new "Host", "Switch"
          end.should_not raise_error
        end
      end


      context "when getting attributes of a link" do
        it "remembers peers" do
          link = Link.new( "Host", "Switch" )
          link.peers.size.should == 2
          link.peers.should include( "Host" )
          link.peers.should include( "Switch" )
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
