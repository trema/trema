#
# Trema paths.
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


module Trema
  def self.home
    File.expand_path File.join( File.dirname( __FILE__ ), "..", ".." )
  end


  def self.objects
    File.join home, "objects"
  end


  def self.cmockery
    File.join objects, "cmockery"
  end


  def self.openvswitch
    File.join objects, "openvswitch"
  end


  def self.cmockery_h
    File.join cmockery, "include/google/cmockery.h"
  end


  def self.libcmockery_a
    File.join cmockery, "lib/libcmockery.a"
  end


  def self.openflow
    File.join objects, "openflow"
  end


  def self.openflow_h
    File.join openflow, "openflow.h"
  end


  def self.oflops
    File.join objects, "oflops"
  end


  def self.tmp
    if ENV.key?( "TREMA_TMP" )
      File.expand_path ENV[ "TREMA_TMP" ]
    else
      File.join home, "tmp"
    end
  end


  def self.log_directory
    File.join tmp, "log"
  end


  module Vendor
    def self.path
      File.join Trema.home, "vendor"
    end


    def self.cmockery
      File.join path, "cmockery-20110428"
    end


    def self.openflow
      File.join path, "openflow-1.0.0"
    end


    def self.openvswitch
      File.join path, "openvswitch-1.2.2"
    end


    def self.phost
      File.join path, "phost"
    end


    def self.oflops
      File.join path, "oflops-0.02"
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
