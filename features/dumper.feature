Feature: dump openflow events with dumper

  As a Trema user
  I want to dump openflow events with dumper application
  So that I can test basic features of trema framework

  Background:
    Given I terminated all trema services


  Scenario: One openflow switch, two servers
    When I try trema run with following configuration:
      """
      vswitch {
        datapath_id "0xabc"
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

      link "0xabc", "host1"
      link "0xabc", "host2"

      app {
        path "./objects/examples/dumper/dumper"
      }
      """
      And wait until "dumper" is up
      And I try to run "./trema send_packets --source host1 --dest host2 -v"
      And I terminated all trema services
    Then the output of trema should include:
      """
      000000010002000000010001080045000032000000004011ff67ffff0001ffff000200010001001e000000000000000000000000000000000000000000000000
      """
