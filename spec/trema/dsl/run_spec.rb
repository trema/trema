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


require File.join( File.dirname( __FILE__ ), "..", "..", "spec_helper" )
require "trema/dsl/run"


module Trema
  module DSL
    describe Run do
      before :each do
        @run = Run.new
      end


      context %[when parsing "run { ... }"] do
        it %[recognizes "path COMMAND_PATH" directive] do
          expect { @run.path "/usr/bin/tremario" }.not_to raise_error
        end


        it %[recognizes "options OPTIONS..." directive] do
          expect { @run.options "--verbose", "--color" }.not_to raise_error
        end
      end


      context "when getting the attributes of an run" do
        it "returns its name" do
          @run.path "/usr/bin/tremario"
          expect( @run[ :name ] ).to eq( "tremario" )
        end


        it "returns its path" do
          @run.path "/usr/bin/tremario"
          expect( @run[ :path ] ).to eq( "/usr/bin/tremario" )
        end


        it "returns its options" do
          @run.options "--verbose", "--color"
          expect( @run[ :options ].size ).to eq( 2 )
          expect( @run[ :options ] ).to include( "--verbose" )
          expect( @run[ :options ] ).to include( "--color" )
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
