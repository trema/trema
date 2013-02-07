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


require "trema/openflow-switch"


module Trema
  #
  # Hardware switch that supports OpenFlow protocol.
  #
  class HardwareSwitch
    #
    # The name of this switch
    #
    # @example
    #   switch.name #=> "My expensive OpenFlow switch"
    #
    # @return [String]
    #
    attr_reader :name


    #
    # Creates a new HardwareSwitch from {DSL::Switch}
    #
    # @example
    #   switch = Trema::HardwareSwitch.new( stanza )
    #
    # @return [HardwareSwitch]
    #
    def initialize stanza
      stanza.validate
      @name = stanza.name
      @stanza = stanza
      OpenflowSwitch.add self
    end


    #
    # Returns datapath id in long format
    #
    # @example
    #   switch.dpid_long #=> "0000000000000abc"
    #
    # @return [String]
    #
    def dpid_long
      @stanza.fetch :dpid_long
    end


    #
    # Returns datapath id prefixed with "0x"
    #
    # @example
    #   switch.dpid_short #=> "0xabc"
    #
    # @return [String]
    #
    def dpid_short
      @stanza.fetch :dpid_short
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
