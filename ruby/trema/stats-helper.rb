#
# Author: Nick Karanatsios <nickkaranatsios@gmail.com>
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
  class StatsHelper

    
    # Called by each StatsReply subclass to set their instance attributes to a value.
    # 
    # @overload initialize(fields, options={})
    # 
    # @param [Array] fields
    #   an array of attribute names to set.
    # 
    # @param [Hash] options
    #   key/value pairs of attributes.
    #   
    # @return [void]
    #
    def initialize fields, options
      fields.each do |field|
        instance_variable_set( "@#{field}", options[field.intern] )
      end
    end

    
    #
    # @return [String] 
    #   an alphabetically sorted text of attribute name/value pairs.
    #
    def to_s
      str = super.to_s + "\n"
      instance_variables.sort.each do |var|
        str += "#{var[1..var.length]}: #{instance_variable_get( var ).to_s}\n"
      end
      str
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End: