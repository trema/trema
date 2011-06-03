Feature: trema dump_flows command

  As a Trema user
  I want to dump the flow table of OpenFlow switch
  So that I can debug my Trema apps

  Background:
    Given I terminated all trema services

  Scenario: run trema dump_flows
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
      And *** sleep 5 ***
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I try to run "./trema dump_flows repeater_hub" (log = "dump_flows.log")
      And I terminated all trema services
    Then "dump_flows.log" should contain some flow entries
