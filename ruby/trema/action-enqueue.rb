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
  # An action to enqueue the packet on the specified queue attached to
  # a port.
  #
  class ActionEnqueue < Action
    attr_reader :port
    attr_reader :queue_id


    #
    # Creates an action to enqueue the packet on the specified queue
    # attached to a port. When a queue is configured the user can
    # associate a flow with this action to forward a packet through
    # the specific queue in that port.
    #
    # @example
    #   ActionEnqueue.new( :port => 1, :queue_id => 2 )
    #
    # @param [Hash] options
    #   the options hash to create this action class instance with.
    #
    # @option options [Number] :port
    #   the port the queue is attached to.
    #
    # @option options [Number] :queue_id
    #   the configured queue. Currently only minimum rate queues provided.
    #
    # @raise [ArgumentError] if both port and queue_id arguments not supplied.
    # @raise [ArgumentError] if port is not an unsigned 16-bit integer.
    # @raise [ArgumentError] if queue id is not an unsigned 32-bit integer.
    # @raise [TypeError] if options is not a Hash.
    #
    def initialize options
      if options.is_a?( Hash )
        @port = options[ :port ]
        @queue_id = options[ :queue_id ]
        if @port.nil?
          raise ArgumentError, ":port is a mandatory option"
        end
        if not @port.unsigned_16bit?
          raise ArgumentError, "Port must be an unsigned 16-bit integer"
        end
        if @queue_id.nil?
          raise ArgumentError, ":queue_id is a mandatory option"
        end
        if not @queue_id.unsigned_32bit?
          raise ArgumentError, ":queue_id must be an unsigned 32-bit integer"
        end
      else
        raise "Invalid option"
      end
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
