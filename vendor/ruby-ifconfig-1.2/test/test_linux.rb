#!/usr/bin/ruby -w

$: << File.dirname(__FILE__) + "/../lib"


require 'ifconfig'
require 'pp'

sample = IO.readlines('../ifconfig_examples/linux.txt').join
ifconfig = IfconfigWrapper.new('Linux',sample).parse

puts "Interfaces: (ifconfig.interfaces)"
pp ifconfig.interfaces

puts "\neth0 mac address: (ifconfig['eth0'].mac)"
pp ifconfig['eth0'].mac

puts "\nIpV4 addresses on eth0: (ifconfig['eth0'].addresses('inet'))"
pp ifconfig['eth0'].addresses('inet')

puts "\nAll addresses reported by ifconfig: (ifconfig.addresses)"
pp ifconfig.addrs_with_type

puts "\niconfig.each { block }"
ifconfig.each do |iface|
  pp iface.name if iface.up?
end

puts
s = ifconfig.to_s
puts s
