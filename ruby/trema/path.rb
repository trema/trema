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


require "trema/monkey-patch/module"


module Trema
  HOME = File.expand_path( File.join( File.dirname( __FILE__ ), "..", ".." ) )


  class << self
    def home
      HOME
    end


    def tmp
      if ENV.key?( "TREMA_TMP" )
        File.expand_path ENV[ "TREMA_TMP" ]
      else
        File.join home, "tmp"
      end
    end


    ############################################################################
    private
    ############################################################################


    def file base, path, name = nil
      define_class_method( name || File.basename( path ).gsub( ".", "_" ) ) do
        File.join __send__( base ), path
      end
    end
    alias :dir :file
  end


  dir :home, "objects"
  dir :home, "ruby"
  dir :home, "src/lib", :include
  dir :home, "vendor"
  dir :objects, "cmockery"
  dir :objects, "lib"
  dir :objects, "oflops"
  dir :objects, "openflow"
  dir :objects, "openvswitch"
  dir :objects, "phost"
  dir :tmp, "log"
  dir :tmp, "pid"
  dir :tmp, "sock"
  dir :vendor, "cmockery-20110428", :vendor_cmockery
  dir :vendor, "oflops-0.03.trema1", :vendor_oflops
  dir :vendor, "openflow-1.0.0", :vendor_openflow
  dir :vendor, "openflow.git", :vendor_openflow_git
  dir :vendor, "openvswitch-1.2.2.trema1", :vendor_openvswitch
  dir :vendor, "phost", :vendor_phost
  dir :vendor, "ruby-ifconfig-1.2", :vendor_ruby_ifconfig
  file :cmockery, "include/google/cmockery.h"
  file :cmockery, "lib/libcmockery.a"
  file :openflow, "openflow.h"
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
