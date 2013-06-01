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
  # An action to modify the IP ToS/DSCP field of a packet.
  #
  class SetIpTos < Action
    # @return [Fixnum] the value of attribute {#type_of_service}
    attr_reader :type_of_service


    #
    # Creates an action to modify the IP ToS/DSCP field of a packet.
    #
    # @example
    #   SetIpTos.new(32)
    #
    # @param [Integer] type_of_service
    #   the ToS/DSCP field to set to.
    #
    # @raise [ArgumentError] if type_of_service value is invalid.
    # @raise [TypeError] if type_of_service argument is not an Integer.
    #
    def initialize type_of_service
      unless type_of_service.is_a?( Integer )
        raise TypeError, "ToS must be an unsigned 8-bit integer"
      end
      unless type_of_service.valid_nw_tos?
        raise ArgumentError, "Invalid type_of_service (ToS) value."
      end
      @type_of_service = type_of_service
    end
  end


  ActionSetNwTos = SetIpTos
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
