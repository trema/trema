#
# Copyright (C) 2008-2012 NEC Corporation
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


require "trema/action"
require "trema/monkey-patch/integer"


module Trema
  #
  # An action to set vendor specific extensions.
  #
  class ActionVendor < Action
    #
    # Creates an action to set vendor specific extensions.
    #
    # @example
    #   ActionVendor.new( 0x00004cff )
    #
    # @param [Integer] vendor
    #   the vendor id this action refers to.
    #
    # @raise [ArgumentError] if vendor argument is not supplied.
    # @raise [ArgumentError] if vendor is not an unsigned 32-bit Integer.
    # @raise [TypeError] if vendor id is not an Integer.
    #
    def initialize vendor
      if not vendor.is_a?( Integer )
        raise TypeError, "Vendor id must be an unsigned 32-bit integer"
      end
      if not vendor.unsigned_32bit?
        raise ArgumentError, "Vendor id must be an unsigned 32-bit integer"
      end
      @value = vendor
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
