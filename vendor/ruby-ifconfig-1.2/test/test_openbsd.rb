#!/usr/bin/ruby -w

$: << File.dirname(__FILE__) + "/../lib"

require 'ifconfig'
require 'pp'

sample = IO.readlines('../ifconfig_examples/openbsd.txt').join
ifconfig = IfconfigWrapper.new('BSD',sample).parse

puts "Interfaces: (ifconfig.interfaces)"
pp ifconfig.interfaces

puts "\nxl0 mac address: (ifconfig['xl0'].mac)"
pp ifconfig['xl0'].mac

puts "\nIpV4 addresses on xl0: (ifconfig['xl0'].addresses('inet'))"
pp ifconfig['xl0'].addresses('inet')

puts "\nAll addresses reported by ifconfig: (ifconfig.addresses)"
pp ifconfig.addrs_with_type

puts "\nList of address types for xl0: (ifconfig['xl0'].addr_types)"
pp ifconfig['xl0'].addr_types

puts "\niconfig.each { block }"
ifconfig.each do |iface|
  pp iface.name if iface.up?
end

puts
s = ifconfig.to_s
puts s
