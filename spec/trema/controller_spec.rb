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


module Trema
  describe Controller do
    class YutaroHub < Controller
    end

    
    context "when running" do
      before :each do
        @yutaro_hub = YutaroHub.new
        @yutaro_hub.stub!( :start_trema )
      end
      

      it "should change its $PROGRAM_NAME" do
        @yutaro_hub.run
        $PROGRAM_NAME.should == "YutaroHub"
      end


      it "should call start callback" do
        @yutaro_hub.should_receive( :start )
        @yutaro_hub.run
      end
    end

    
    it "should respond to logging methods" do
      [ :critical, :error, :warn, :notice, :info, :debug ].each do | each |
        Controller.new.__send__( each, "%s message", each ).should == "#{ each } message"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:

