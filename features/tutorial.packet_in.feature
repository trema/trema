Feature: Tutorial: Handling packet_in events example

  As a Trema user
  I want to respond to packet_in events
  So that I can handle unknown packets

  Scenario: Handle packet_in
    When I try trema run with following configuration:
      """
      vswitch { dpid "0xabc" }

      vhost {
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }
      vhost {
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }

      link "0xabc", "192.168.0.1"
      link "0xabc", "192.168.0.2"

      app {
        path "./objects/examples/packet_in/packet_in"
      }
      """
      And *** sleep 10 ***
      And I try to run "./trema send_packets --source 192.168.0.1 --dest 192.168.0.2"
      And I terminated all trema services
    Then the output of trema should include:
      """
      000000010002000000010001080045000032000000004011ff67ffff0001ffff000200010001001e000000000000000000000000000000000000000000000000
      """
