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


module Trema
  #
  # As physical ports of the datapath are modified, the controller
  # needs to be informed with this message.
  #
  class PortStatusModify < PortStatus
    #
    # Creates a port-modified message.
    #
    # @example
    #   PortStatusModify.new(
    #     :datapath_id => 0xabc,
    #     :transaction_id => 123,
    #     :phy_port => port
    #   )
    #
    # @param [Hash] options
    #   the options to create a message with.
    #
    # @option options [Number] :datapath_id
    #   message originator identifier.
    #
    # @option options [Number] :transaction_id
    #   unsolicited message transaction_id is zero.
    #
    # @option options [Port] :phy_port
    #   a {Port} object describing the properties of the port.
    #
    # @return [PortStatusModify]
    #
    def initialize options
      super options.merge( :reason => OFPPR_MODIFY )
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8
### indent-tabs-mode: nil
### End:
