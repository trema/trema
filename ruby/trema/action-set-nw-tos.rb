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
  # An action to modify the IP ToS/DSCP field of a packet.
  #
  class ActionSetNwTos < Action
    attr_reader :tos


    #
    # Creates an action to modify the IP ToS/DSCP field of a packet.
    #
    # @example
    #   ActionSetNwTos.new(32)
    #
    # @param [Integer] :tos
    #   the ToS/DSCP field to set to.
    #
    # @raise [ArgumentError] if nw_tos argument is not an unsigned 8-bit Integer.
    # @raise [TypeError] if supplied argument is not an Integer.
    #
    def initialize tos
      if not tos.is_a?( Integer )
        raise TypeError, "ToS must be an unsigned 8-bit integer"
      end
      if not tos.unsigned_8bit?
        raise ArgumentError, "ToS must be an unsigned 8-bit integer"
      end
      @tos = tos
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
