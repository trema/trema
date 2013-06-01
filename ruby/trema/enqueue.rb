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
  # An action to enqueue the packet on the specified queue attached to
  # a port.
  #
  class Enqueue < Action
    attr_reader :port_number
    attr_reader :queue_id


    #
    # Creates an action to enqueue the packet on the specified queue
    # attached to a port. When a queue is configured the user can
    # associate a flow with this action to forward a packet through
    # the specific queue in that port.
    #
    # @example
    #   Enqueue.new( :port_number => 1, :queue_id => 2 )
    #
    # @param [Hash] options
    #   the options hash to create this action class instance with.
    #
    # @option options [Number] :port_number
    #   the port the queue is attached to.
    #
    # @option options [Number] :queue_id
    #   the configured queue. Currently only minimum rate queues provided.
    #
    # @raise [TypeError] if options is not a Hash.
    # @raise [ArgumentError] if both port_number and queue_id arguments not supplied.
    # @raise [ArgumentError] if port_number is not an unsigned 16-bit integer.
    # @raise [ArgumentError] if queue id is not an unsigned 32-bit integer.
    #
    def initialize options
      if options.is_a?( Hash )
        @port_number = options[ :port_number ]
        @queue_id = options[ :queue_id ]
        if @port_number.nil?
          raise ArgumentError, "Port number is a mandatory option"
        end
        unless @port_number.unsigned_16bit?
          raise ArgumentError, "Port number must be an unsigned 16-bit integer"
        end
        if @queue_id.nil?
          raise ArgumentError, "Queue ID is a mandatory option"
        end
        unless @queue_id.unsigned_32bit?
          raise ArgumentError, "Queue ID must be an unsigned 32-bit integer"
        end
      else
        raise "Invalid option"
      end
    end
  end


  ActionEnqueue = Enqueue
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
