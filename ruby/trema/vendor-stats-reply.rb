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
  class VendorStatsReply < StatsHelper
    FIELDS = %w(vendor_id data)
    FIELDS.each { |field| attr_reader field.intern }


    # Vendor statistics reply.
    # A user would not explicitly instantiate a {VendorStatsReply} object but would
    # be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_VENDOR)+
    # openflow message.
    #
    # @overload initialize(options={})
    #
    #   @example
    #     VendorStatsReply.new(
    #       :vendor_id => 123,
    #       :data => "deadbeef".unpack( "C*" )
    #     )
    #
    #   @param [Hash] options
    #     the options to create this instance with.
    #
    #   @option options [Number] :vendor_id
    #     the specific vendor identifier.
    #
    #   @option options [Array] :data
    #     a String that holds vendor's defined arbitrary length data.
    #
    #   @return [VendorStatsReply]
    #     an object that encapsulates the OFPST_STATS_REPLY(OFPST_VENDOR) OpenFlow message.
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
