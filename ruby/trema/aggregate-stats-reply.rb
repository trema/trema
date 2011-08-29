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


require "trema/stats-helper"


module Trema
  class AggregateStatsReply < StatsHelper
    FIELDS = %w( packet_count byte_count flow_count )
    FIELDS.each { | each | attr_reader each.intern }


    #
    # Creates an AggregateStatsReply object from options hash.
    #
    # @example AggregateStatsReply.new( options = {} )
    #   options = { :packet_count => 1, :byte_count => 64, :flow_count => 1 }
    #
    # @return [AggregateStatsReply] an object that encapsulates the OFPST_STATS_REPLY (OFPST_AGGREGATE) OpenFlow message.
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
