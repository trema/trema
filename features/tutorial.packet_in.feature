Feature: Tutorial: Handling packet_in events example

  As a Trema user
  I want to respond to packet_in events
  So that I can handle unknown packets

  Scenario: Handle packet_in
    When I try trema run "./objects/examples/packet_in/packet_in" with following configuration (backgrounded):
      """
      vswitch("pktin") {
        dpid "0xabc"
      }

      vhost("host1") {
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }

      vhost("host2") {
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }

      link "pktin", "host1"
      link "pktin", "host2"
      """
      And wait until "packet_in" is up
      And *** sleep 5 ***
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I terminated all trema services
    Then the output should include:
      """
      000000010002000000010001080045000032000000004011ff67ffff0001ffff000200010001001e000000000000000000000000000000000000000000000000
      """

  Scenario: Handle packet_in in Ruby
    When I try trema run "./src/examples/packet_in/packet_in.rb" with following configuration (backgrounded):
      """
      vswitch("pktin") {
        dpid "0xabc"
      }

      vhost("host1") {
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }

      vhost("host2") {
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }

      link "pktin", "host1"
      link "pktin", "host2"
      """
      And wait until "PacketinDumper" is up
      And *** sleep 5 ***
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I terminated all trema services
    Then the output should include:
      """
      received a packet_in
      """
