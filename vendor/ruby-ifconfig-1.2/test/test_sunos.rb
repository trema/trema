#!/usr/bin/ruby -w

$: << File.dirname(__FILE__) + "/../lib"

#
#imaptest1# netstat -in
#Name  Mtu  Net/Dest      Address        Ipkts  Ierrs Opkts  Oerrs Collis Queue 
#lo0   8232 0.0.0.0       0.0.0.4        267824 0     267824 0     0      0     
#bge0  1500 10.0.0.0      10.32.4.138    10935939 0     7741167 0     0      0   
require 'ifconfig'
require 'pp'

sample = IO.readlines('../ifconfig_examples/sunos.txt').join
ifconfig = IfconfigWrapper.new('SunOS',sample).parse

puts "Interfaces: (ifconfig.interfaces)"
pp ifconfig.interfaces

puts "\nbge0 mac address: (ifconfig['bge0'].mac)"
pp ifconfig['bge0'].mac

puts "\nIpV4 addresses on bge0: (ifconfig['bge0'].addresses('inet'))"
pp ifconfig['bge0'].addresses('inet')

puts "\nAll addresses reported by ifconfig: (ifconfig.addresses)"
pp ifconfig.addrs_with_type

puts "\niconfig.each { block }"
ifconfig.each do |iface|
  pp iface.name if iface.up?
end

puts
s = ifconfig.to_s
puts s
