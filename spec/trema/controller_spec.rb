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


module Trema
  describe Controller do
    subject { Controller }

    it { should have_constant :OFPP_MAX }
    it { should have_constant :OFPP_IN_PORT }
    it { should have_constant :OFPP_TABLE }
    it { should have_constant :OFPP_NORMAL }
    it { should have_constant :OFPP_FLOOD }
    it { should have_constant :OFPP_ALL }
    it { should have_constant :OFPP_CONTROLLER }
    it { should have_constant :OFPP_LOCAL }
    it { should have_constant :OFPP_NONE }


    context ".new" do
      subject { Controller.new }

      it { should respond_to :critical }
      it { should respond_to :error }
      it { should respond_to :warn }
      it { should respond_to :notice }
      it { should respond_to :info }
      it { should respond_to :debug }
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
