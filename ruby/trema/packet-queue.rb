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
  class Queue
    class << self
      #
      # The {PacketQueue} to append to the list.
      #
      attr_accessor :queues
    end


    # Add queue to list.
    # @param [PacketQueue] queue a {PacketQueue} instance.
    def self.append queue
      @queues ||= []
      @queues << queue unless @queues.include?(queue)
    end


    # @return [Array]
    #   an array of {PacketQueue} objects.
    def self.queues
      @queues
    end


    # Iterate over each {PacketQueue} item.
    # @return [Array] a list of {PacketQueue} items.
    def self.each &block
      @queues.each do | each |
        block.call each
      end
    end
  end


  class PacketQueue
    # No property for queue.
    OFPQT_NONE = 0

    # Minimum datarate guaranteed.
    OFPQT_MIN_RATE = 1

    # Id for the specific queue.
    # @return [Number] queue_id
    #   the value of attribute queue_id.
    attr_accessor :queue_id

    # Queue description's length in bytes.
    # @return [Number] len
    #   the value of attribute len.
    attr_accessor :len

    # List of queue properties.
    # @return [Array] properties
    #   the value of attribute properties.
    attr_accessor :properties


    # @overload initialize(options={})
    # @param [Hash] options ths options hash.
    # @option options [Symbol] :queue_id
    #   id for the specific queue.
    # @option options [Symbol] :len
    #   queue description's length in bytes.
    #
    def initialize options
      @queue_id = options[ :queue_id ]
      @len = options[ :len ]
      @properties = []
    end


    # @param [MinRateQueue] queue
    #   a property queue {MinRateQueue} object to append to the properties list.
    def append queue
      @properties << queue
    end


    # @return [String]
    #   text representation of {PacketQueue}'s attributes and all its properties
    #   queue object's attributes.
    def to_s
      str = "queue_id: #{@queue_id} len: #{@len} "
      @properties.each do | each |
        str += each.to_s
      end
      str
    end
  end


  class QueueProperty
    # Property queue id.
    # For minimum-rate type queue the property value is set to 1, otherwise defaults
    # to zero.
    # @return [Number]
    #   the value of attribute property.
    attr_accessor :property

    # length of property. If >= 8 minimum-rate type queue is defined.
    # @return [Number]
    #   the value of attribute len.
    attr_accessor :len


    # Each queue is described by a set of properties, each of a specific type and
    # configuration.
    # @param [Number] property
    #   property queue id.
    # @param [Number] len
    #   length in bytes of the property queue.
    def initialize property, len
      @property = property
      @len = len
    end


    # @return [String]
    #  text representation of its attributes (property,len).
    def to_s
      "property: #{@property} len: #{@len} "
    end
  end


  class MinRateQueue < QueueProperty
    # the rate value of the minimum rate queue.
    # @return [Number]
    #   the value of attribute rate.
    attr_accessor :rate


    # An object that encapsulates the minimum rate queue property description.
    # @param [Number] property
    #   property queue id.
    # @param [Number] len
    #   length in bytes of the property queue.
    # @param [Number] rate
    #   the rate value of the mimimum rate queue.
    # @param [PacketQueue] packet_queue
    #   A {PacketQueue} instance to use to save the minimum rate queue.
    def initialize property, len, rate, packet_queue
      super property, len
      @rate = rate
      packet_queue.append self
      Trema::Queue.append packet_queue
    end


    # @return [String]
    #  text representation of rate prefixed by property and length attributes.
    def to_s
      super.to_s + " rate: #{rate} "
    end
  end
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
