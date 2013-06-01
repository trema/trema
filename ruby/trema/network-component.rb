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


require "trema/ordered-hash"


module Trema
  #
  # The base class of objects appears in the Trema DSL. e.g., host,
  # switch, link etc.
  #
  class NetworkComponent
    class << self
      #
      # Returns the `name' => object hash DB of instances
      #
      # @example
      #   p App.instances
      #   #=> {"trema tetris"=>#<App:0xb73c9328>, ...}
      #
      # @return [Array] the {OrderedHash} of instances
      #
      # @api public
      #
      attr_accessor :instances
    end


    #
    # Called implicitly when inherited
    #
    # @example
    #   #
    #   # The following calls inherited() implicitly
    #   # then creates an instance DB of App object.
    #   #
    #   class App < Trmea::NetworkComponent
    #     attr_accessor :name
    #   end
    #
    # @return [undefined]
    #
    # @api public
    #
    def self.inherited subclass
      subclass.instances ||= OrderedHash.new
    end



    def self.clear
      instances.clear
    end


    #
    # Iterates over the list of instances
    #
    # @example
    #   App.each do | each |
    #     p each.name
    #   end
    #
    # @return [Array] the list of instances
    #
    # @api public
    #
    def self.each &block
      instances.values.each do | each |
        block.call each
      end
    end


    #
    # Looks up a instance DB by its name
    #
    # @example
    #   ttetris = TremaTetris.new
    #   ttetris.name = "trema tetris"
    #
    #   App.add ttetris
    #
    #   App[ "trema tetris" ] => ttetris
    #
    # @return [Object] the associated object
    #
    # @api public
    #
    def self.[] name
      instances[ name ]
    end


    #
    # Returns the number of instances.
    #
    # @example
    #   App.size  #=> 3
    #
    # @return [Number] the number of instances
    #
    # @api public
    #
    def self.size
      instances.values.size
    end


    #
    # Inserts a object to instance DB
    #
    # @example
    #   ttetris = TremaTetris.new
    #   ttetris.name = "trema tetris"
    #
    #   App.add ttetris
    #
    # @param [Object, #name] object
    #
    # @return [Object] the added object
    #
    # @api public
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
