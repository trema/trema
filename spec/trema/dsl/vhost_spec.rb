#
# Author: Yasuhito Takamiya <yasuhito@gmail.com>
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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )
require "trema/dsl/vhost"


module Trema
  module DSL
    describe Vhost do
      before :each do
        @vhost = Vhost.new
      end


      context %[when parsing "vhost { ... }"] do
        it %[recognizes "ip IP_ADDRESS" directive] do
          lambda do
            @vhost.ip "192.168.0.1"
          end.should_not raise_error
        end


        it %[recognizes "netmask NETMASK" directive] do
          lambda do
            @vhost.netmask "255.255.255.0"
          end.should_not raise_error
        end


        it %[recognizes "mac MAC_ADDRESS" directive] do
          lambda do
            @vhost.mac "00:00:00:01:00:01"
          end.should_not raise_error
        end
      end


      context "when getting the attributes of a vhost" do
        it "returns its name" do
          @vhost.ip "192.168.0.1"
          @vhost[ :name ].should == "192.168.0.1"
        end


        it "returns its IP address" do
          @vhost.ip "192.168.0.1"
          @vhost[ :ip ].should == "192.168.0.1"
        end


        it "returns its netmask address" do
          @vhost.netmask "255.255.255.0"
          @vhost[ :netmask ].should == "255.255.255.0"
        end


        it "returns its MAC address" do
          @vhost.mac "00:00:00:01:00:01"
          @vhost[ :mac ].should == "00:00:00:01:00:01"
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
