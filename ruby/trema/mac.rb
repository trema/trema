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
  class Mac
    attr_reader :value


    def initialize value
      if value.is_a?( String )
        if /^([0-9a-fA-F][0-9a-fA-F]:){5}([0-9a-fA-F][0-9a-fA-F])$/=~ value
          @value = eval( "0x" + value.gsub( ":", "" ) )
        else
          raise %{Invalid MAC address: "#{ value }"}
        end
      else
        if value >= 0 and value <= 0xffffffffffff
          @value = value
        else
          raise %{Invalid MAC address: #{ value }}
        end
      end
      @string = string_format
    end


    def to_s
      @string
    end

    
    def to_short
      @string.split( ':' ).collect { |each| each.hex }
    end
    
    
    def == other
      @value == other.value
    end


    ################################################################################
    private
    ################################################################################


    def string_format
      if @value.kind_of? Integer
        v = sprintf( "%012x", @value ).split( // )
        [
          v[ 0 ], v[ 1 ], ":",
          v[ 2 ], v[ 3 ], ":",
          v[ 4 ], v[ 5 ], ":",
          v[ 6 ], v[ 7 ], ":",
          v[ 8 ], v[ 9 ], ":",
          v[ 10 ], v[ 11 ]
        ].join
      else
        v = @value
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
