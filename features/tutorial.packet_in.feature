Feature: Tutorial: Handling packet_in events example

  As a Trema user
  I want to respond to packet_in events
  So that I can handle unknown packets


  Scenario: Handle packet_in
    When I try trema run "./objects/examples/packet_in/packet_in" with following configuration (backgrounded):
      """
      vswitch("packet_in") { dpid "0xabc" }

      vhost("host1")
      vhost("host2")

      link "packet_in", "host1"
      link "packet_in", "host2"
      """
      And wait until "packet_in" is up
      And I try to run "./trema send_packets --source host1 --dest host2"
      And *** sleep 2 ***
      And I terminated all trema services
    Then the output should include:
      """
      received a packet_in
      """


  Scenario: Handle packet_in in Ruby
    When I try trema run "./src/examples/packet_in/packet-in.rb" with following configuration (backgrounded):
      """
      vswitch("packet_in") { dpid "0xabc" }

      vhost("host1")
      vhost("host2")

      link "packet_in", "host1"
      link "packet_in", "host2"
      """
      And wait until "PacketinDumper" is up
      And I try to run "./trema send_packets --source host1 --dest host2"
      And *** sleep 2 ***
      And I terminated all trema services
    Then the output should include:
      """
      received a packet_in
      """
