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
require "trema/dsl/app"


module Trema
  module DSL
    describe App do
      before :each do
        @app = App.new
      end


      context %[when parsing "app { ... }"] do
        it %[recognizes "path COMMAND_PATH" directive] do
          lambda do
            @app.path "/usr/bin/tremario"
          end.should_not raise_error
        end


        it %[recognizes "options OPTIONS..." directive] do
          lambda do
            @app.options "--verbose", "--color"
          end.should_not raise_error
        end
      end


      context "when getting the attributes of an app" do
        it "returns its name" do
          @app.path "/usr/bin/tremario"
          @app[ :name ].should == "tremario"
        end


        it "returns its path" do
          @app.path "/usr/bin/tremario"
          @app[ :path ].should == "/usr/bin/tremario"
        end


        it "returns its options" do
          @app.options "--verbose", "--color"
          @app[ :options ].size.should == 2
          @app[ :options ].should include( "--verbose" )
          @app[ :options ].should include( "--color" )
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
