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


require File.join( File.dirname( __FILE__ ), "..", "spec_helper" )
require "trema"


describe Trema::Controller do
  it "should send hello messages to switches" do
    Trema::Controller.new.send_message Trema::Hello.new( 1 ), 0xabc
  end


  it "should call start() on startup" do
    class MyController < Trema::Controller
      def start
        raise "started"
      end
    end

    lambda do
      MyController.new.run
    end.should raise_error( "started" )
  end


  it "should respond to logging methods" do
    [ :critical, :error, :warn, :notice, :info, :debug ].each do | each |
      Trema::Controller.new.should respond_to( each )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:

