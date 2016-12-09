class ARPEntry
  attr_reader :port
  attr_reader :hwaddr
  attr_writer :age_max

  def initialize( port, hwaddr, age_max )
    @port = port
    @hwaddr = hwaddr
    @age_max = age_max
    @last_updated = Time.now
   p "New entry: MAC addr = #{ @hwaddr.to_s }, port = #{ @port }"
  end

  def update( port, hwaddr )
    @port = port
    @hwaddr = hwaddr
    @last_updated = Time.now
    p "Update entry: MAC addr = #{ @hwaddr.to_s }, port = #{ @port }"
  end

  def aged_out?()
    aged_out = Time.now - @last_updated > @age_max
    logger.info "Age out: An ARP entry (MAC address = #{ @hwaddr.to_s }, port number = #{ @port }) has been aged-out" if aged_out
    aged_out
  end
end


class ARPTable
  DEFAULT_AGE_MAX = 300

  def initialize()
    @db = {}
  end

  def update( port, ipaddr, hwaddr )
    entry = @db[ ipaddr.to_i ]
    if entry
      entry.update( port, hwaddr )
    else
      new_entry = ARPEntry.new( port, hwaddr, DEFAULT_AGE_MAX )
      @db[ ipaddr.to_i ] = new_entry
    end
  end

  def lookup( ipaddr )
    @db[ ipaddr.to_i ]
  end

  def age()
    @db.delete_if do | ipaddr, entry |
      entry.aged_out?
    end
  end
end
