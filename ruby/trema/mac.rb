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
  #
  # MAC address class
  #
  class Mac
    attr_reader :value


    def initialize value
      case value
      when String
        @value = value_from_string( value )
      when Integer
        @value = value_from_integer( value )
      else
        raise %{Invalid MAC address: #{ value }}
      end
      @string = string_format
    end


    def to_s
      @string
    end

    
    def to_short
      @string.split( ":" ).collect do | each |
        each.hex
      end
    end
    
    
    def == other
      @value == other.value
    end


    ################################################################################
    private
    ################################################################################


    def value_from_string string
      if /^([0-9a-fA-F][0-9a-fA-F]:){5}([0-9a-fA-F][0-9a-fA-F])$/=~ string
        eval( "0x" + string.gsub( ":", "" ) )
      else
        raise %{Invalid MAC address: "#{ string }"}
      end
    end


    def value_from_integer integer
      if integer >= 0 and integer <= 0xffffffffffff
        integer
      else
        raise %{Invalid MAC address: #{ integer }}
      end
    end


    def string_format
      sprintf( "%012x", @value ).unpack( "a2" * 6 ).join( ":" )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
