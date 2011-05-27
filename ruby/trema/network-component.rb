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


require "trema/ordered-hash"


module Trema
  class NetworkComponent
    class << self
      attr_accessor :instances
    end


    def self.inherited subclass
      subclass.instances ||= OrderedHash.new
    end


    def self.instance
      instances.values[ 0 ]
    end
    
    
    #
    # Iterates over the list of instances.
    #
    def self.each &block
      instances.values.each do | each |
        block.call each
      end
    end


    #
    # Looks up a instance DB by its name.
    #
    def self.[] name
      instances[ name ]
    end


    #
    # Inserts a object to instance DB.
    #
    def self.add object
      instances[ object.name ] = object
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
