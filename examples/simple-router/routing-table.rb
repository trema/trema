require "pio"

class RoutingTable
  ADDR_LEN = 32

  def initialize( route = [] )
    @db = Array.new( ADDR_LEN + 1 ) { Hash.new }
    route.each do | each |
      add( each )
    end
  end

  def add( options )
    dest = IPAddr.new( options[ :destination ] )
    masklen = options[ :masklen ]
    prefix = dest.mask( masklen )
    @db[ masklen ][ prefix.to_i ] = IPAddr.new( options[ :nexthop ] )
  end

  def delete( options )
    dest = IPAddr.new( options[ :destination ] )
    masklen = options[ :masklen ]
    prefix = dest.mask( masklen )
    @db[ masklen ].delete( prefix.to_i )
  end

  def lookup( dest )
    ( 0..ADDR_LEN ).reverse_each do | masklen |
      prefix = dest.mask( masklen )
      entry = @db[ masklen ][ prefix.to_i ]
      return entry if entry
    end
    nil
  end
end
