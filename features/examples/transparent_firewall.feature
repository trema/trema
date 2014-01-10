Feature: "Transparent Firewall" sample application

  @slow_process
  Scenario: block-rfc1918.rb blocks packets with private source/destination address
    Given a file named "transparent_firewall.conf" with:
      """
      vswitch("firewall") { datapath_id "0xabc" }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "firewall", "host1"
      link "firewall", "host2"
      """
    And I run `trema run ../../src/examples/transparent_firewall/block-rfc1918.rb -c transparent_firewall.conf -d`
    And wait until "BlockRFC1918" is up
    When I send 1 packet from host1 to host2
    And I run `trema show_stats host2 --rx`
    And the output should not contain "192.168.0.2,1,192.168.0.1,1,1,50"

  @slow_process
  Scenario: block-rfc1918.rb does not block packets with global source/destination address
    Given a file named "transparent_firewall.conf" with:
      """
      vswitch("firewall") { datapath_id "0xabc" }

      vhost("host1") { ip "1.1.1.1" }
      vhost("host2") { ip "2.2.2.2" }

      link "firewall", "host1"
      link "firewall", "host2"
      """
    And I run `trema run ../../src/examples/transparent_firewall/block-rfc1918.rb -c transparent_firewall.conf -d`
    And wait until "BlockRFC1918" is up
    When I send 1 packet from host1 to host2
    And I run `trema show_stats host2 --rx`
    And the output should contain "2.2.2.2,1,1.1.1.1,1,1,50"
