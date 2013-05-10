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


require "trema/action"
require "trema/monkey-patch/integer"


module Trema
  #
  # An action to set vendor specific extensions.
  #
  class VendorAction < Action
    #
    # @return [Array<Fixnum>] the value of attribute {#body} that represents
    #   binary data as an array of bytes.
    #
    attr_reader :body
    # @return [Integer] the value of attribute {#vendor_id}
    attr_reader :vendor_id


    #
    # Creates an action to set vendor specific extensions.
    #
    # @example
    #   VendorAction.new( 0x00004cff, "deadbeef".unpack( "C*" ) )
    #
    # @param [Integer] vendor_id
    #   the vendor identifier. If MSB is zero low order bytes are IEEE
    #   OUI. Otherwise defined by openflow.
    # @param [Array] body
    #   vendor-defined arbitrary additional data.
    #
    # @raise [TypeError] if vendor ID is not an Integer.
    # @raise [ArgumentError] if vendor ID is not an unsigned 32-bit Integer.
    # @raise [TypeError] if body is not an Array.
    # @raise [ArgumentError] if body length is not a multiple of 8.
    #
    def initialize vendor_id, body = nil
      unless vendor_id.is_a?( Integer )
        raise TypeError, "Vendor ID must be an unsigned 32-bit integer"
      end
      unless vendor_id.unsigned_32bit?
        raise ArgumentError, "Vendor ID must be an unsigned 32-bit integer"
      end
      if ( not body.nil? ) and ( not body.is_a?( Array ) )
        raise TypeError, "Body must be an Array"
      end
      if ( not body.nil? ) and ( body.size % 8 != 0 )
        raise ArgumentError, "Body length must be a multiple of 8"
      end

      @vendor_id = vendor_id
      @body = body
    end
  end


  ActionVendor = VendorAction
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
