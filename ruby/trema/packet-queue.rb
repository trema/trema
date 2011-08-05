require "singleton"


class Queue
  include Singleton
  
  
  def append queue
    @@queues ||= []
    @@queues << queue
  end
  
  
  def queues
    @@queues
  end
end


QUEUE = Queue.instance


class PacketQueue
  attr_accessor :queue_id, :len, :properties


  def initialize options
    @queue_id = options[ :queue_id ]
    @len = options[ :len ]
    @properties = []
  end


  def append queue
    @properties << queue
  end


  def to_s
    str = "queue_id: #{@queue_id} len: #{@len} "
    @properties.each do | each |
      str += each.to_s
    end
    str
  end
end


class QueueProperty
  attr_accessor :property, :len


  def initialize property, len
    @property = property
    @len = len
  end


  def to_s
    "property: #{@property} len: #{@len} "
  end
end


class MinRateQueue < QueueProperty
  attr_accessor :rate

  
  def initialize property, len, rate, packet_queue
    super property, len
    @rate = rate
    packet_queue.append self
    QUEUE.append packet_queue
  end


  def to_s
    super.to_s + " rate: #{rate} "
  end
end
