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


require "trema/stats-helper"


module Trema
  class DescStatsReply < StatsHelper
    FIELDS = %w(mfr_desc hw_desc sw_desc serial_num dp_desc )
    FIELDS.each { |field| attr_reader field.intern }


    # Descriptive information about a vswitch.
    # A user would not explicitly instantiate a {DescStatsReply} object but
    # would be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_DESC)+
    # message.
    #
    # @overload initialize(options={})
    #
    #   @example
    #     DescStatsReply.new(
    #       :mfr_desc => "Nicira Networks, Inc.",
    #       :hw_desc => "Open vSwitch",
    #       :sw_desc => "1.2.2"
    #       :serial_num => "None"
    #     )
    #
    #   @param [Hash] options
    #     the options to create this instance with.
    #
    #   @option options [String] :mfr_desc
    #     the manufacturer description.
    #
    #   @option options [String] :hw_desc
    #     the hardware description.
    #
    #   @option options [String] :sw_desc
    #     the software description.
    #
    #   @option options [String] :serial_num
    #     the serial number.
    #
    #   @option options [String] :dp_desc
    #     the human readable description of datapath.
    #
    #   @return [DescStatsReply]
    #     an object that encapsulates the OFPST_STATS_REPLY(OFPST_DESC) OpenFlow message.
    #
    def initialize options
      super FIELDS, options
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
