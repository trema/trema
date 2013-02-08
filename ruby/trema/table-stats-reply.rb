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
  class TableStatsReply < StatsHelper
    FIELDS = %w(table_id name wildcards max_entries ) +
      %w(active_count lookup_count matched_count)
    FIELDS.each { |field| attr_reader field.intern }


    # Information about tables that a switch supports. A switch may choose to
    # maintain a single table that can store both wildcard and exact match flows.
    # Or may use separate tables for each flow type.
    # A user would not explicitly instantiate a {TableStatsReply} object but would
    # be created as a result of parsing the +OFPT_STATS_REPLY(OFPST_TABLE)+ message.
    #
    # @overload initialize(options={})
    #
    #   @example
    #     TableStatsReply.new(
    #       :table_id => 0,
    #       :name => "classifier",
    #       :wildcards => 4194303,
    #       :max_entries => 1048576,
    #       :active_count => 4,
    #       :lookup_count => 4,
    #       :matched_count => 0
    #     )
    #
    #   @param [Hash] options
    #     the options to create this instance with.
    #
    #   @option options [Number] :table_id
    #     a number that uniquely identifies the table.
    #
    #   @option options [String] :name
    #     a meaningful name for the table.
    #
    #   @option options [Number] :wildcards
    #     wildcards that supported by the table.
    #
    #   @option options [Number] :max_entries
    #     the maximum number of flow entries the table can support.
    #
    #   @option options [Number] :active_count
    #     number of active entries.
    #
    #   @option options [Number] :lookup_count
    #     a counter of the number of packets looked up in the table.
    #
    #   @option options [Number] :matched_count
    #     a counter of the matched hit entries.
    #
    #   @return [TableStatsReply]
    #     an object that encapsulates the OFPST_STATS_REPLY(OFPST_TABLE) OpenFlow message.
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
