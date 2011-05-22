Feature: Control one openflow switch with repeater_hub controller

  As a Trema user
  I want to control one openflow switch using repeater_hub controller
  So that I can send and receive packets

  Background:
    Given I terminated all trema services

  Scenario: Run repeater hub with one openflow switch and three servers
    When I try trema run "./objects/examples/repeater_hub/repeater_hub" with following configuration (backgrounded):
      """
      vswitch("repeater_hub") {
        datapath_id "0xabc"
      }

      vhost("host1") {
        promisc "On"
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }

      vhost("host2") {
        promisc "On"
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }

      vhost("host3") {
        promisc "On"
        ip "192.168.0.3"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:03"
      }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
      """
      And wait until "repeater_hub" is up
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I try to run "./trema show_stats host1 --tx" (log = "host1.repeater_hub.log")
      And I try to run "./trema show_stats host2 --rx" (log = "host2.repeater_hub.log")
      And I try to run "./trema show_stats host3 --rx" (log = "host3.repeater_hub.log")
      And I terminated all trema services
    Then the content of "host1.repeater_hub.log" and "host2.repeater_hub.log" should be identical
     And the content of "host1.repeater_hub.log" and "host3.repeater_hub.log" should be identical

  Scenario: Run repeater hub (Ruby) with one openflow switch and three servers
    When I try trema run "./src/examples/repeater_hub/repeater-hub.rb" with following configuration (backgrounded):
      """
      vswitch("repeater_hub") {
        datapath_id "0xabc"
      }

      vhost("host1") {
        promisc "On"
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }

      vhost("host2") {
        promisc "On"
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }

      vhost("host3") {
        promisc "On"
        ip "192.168.0.3"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:03"
      }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
      """
      And wait until "RepeaterHub" is up
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I try to run "./trema show_stats host1 --tx" (log = "host1.repeater-hub.rb.log")
      And I try to run "./trema show_stats host2 --rx" (log = "host2.repeater-hub.rb.log")
      And I try to run "./trema show_stats host3 --rx" (log = "host3.repeater-hub.rb.log")
      And I terminated all trema services
    Then the content of "host1.repeater-hub.rb.log" and "host2.repeater-hub.rb.log" should be identical
     And the content of "host1.repeater-hub.rb.log" and "host3.repeater-hub.rb.log" should be identical
