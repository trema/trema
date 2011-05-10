Feature: Dump openflow events with dumper

  As a Trema user
  I want to dump OpenFlow events with dumper application
  So that I can visualize OpenFlow messages

  Background:
    Given I terminated all trema services


  Scenario: One openflow switch and two servers
    When I try trema run with following configuration (log = "dumper.log"):
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
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I terminate all trema services
    Then the log file "dumper.log" should include:
      """
      000000010002000000010001080045000032000000004011ff67ffff0001ffff000200010001001e000000000000000000000000000000000000000000000000
      """
