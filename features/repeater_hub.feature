Feature: Control one openflow switch with repeater_hub controller

  As a Trema user
  I want to control one openflow switch using repeater_hub controller
  So that I can send and receive packets

  Background:
    Given I terminated all trema services

  Scenario: One openflow switch, two servers
    When I try trema run with following configuration:
      """
      vswitch("repeater_hub") {
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

      vhost("host3") {
        ip "192.168.0.3"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:03"
      }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"

      app {
        path "./objects/examples/repeater_hub/repeater_hub"
      }
      """
      And wait until "repeater_hub" is up
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I try to run "./trema show_stats host1 --tx > ./tmp/log/host1.repeater_hub.log"
      And I try to run "./trema show_stats host2 --rx > ./tmp/log/host2.repeater_hub.log"
      And I try to run "./trema show_stats host3 --rx > ./tmp/log/host3.repeater_hub.log"
      And I terminated all trema services
    Then the content of "./tmp/log/host1.repeater_hub.log" and "./tmp/log/host2.repeater_hub.log" should be identical
     And the content of "./tmp/log/host1.repeater_hub.log" and "./tmp/log/host3.repeater_hub.log" should be identical
