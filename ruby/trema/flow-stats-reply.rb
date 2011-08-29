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
  class FlowStatsReply < StatsHelper
    FIELDS = %w( length table_id match duration_sec duration_nsec priority idle_timeout hard_timeout cookie packet_count byte_count actions )
    FIELDS.each { | each | attr_reader each.intern }


    #
    # Creates a FlowStatsReply object from options hash.
    #
    # @example FlowStatsReply.new( options = {} )
    #   options { :length => 86, :table_id => 1, :match => Match.new
    #   :duration_sec => 10, :duration_nsec => 555, :priority => 0, :idle_timeout => 0,
    #   :hard_timeout => 0, :cookie => 0xabcd, :packet_count => 1, :byte_count => 1
    #   :actions => [ ActionOutput.new ] }
    #
    # @return [FlowStatsReply] an object that encapsulates the OFPST_STATS_REPLY (OPPST_FLOW) OpenFlow message.
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
