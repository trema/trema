Feature: "Repeater Hub" sample application

  In order to learn how to flood incoming packets to every other port
  As a developer using Trema
  I want to execute "Repeater Hub" sample application

  Background:
    Given a file named "repeater_hub.conf" with:
      """
      vswitch("repeater_hub") { datapath_id 0xabc }

      vhost("host1") {
        ip "192.168.0.1"
        promisc "On"
      }
      vhost("host2") {
        ip "192.168.0.2"
        promisc "On"
      }
      vhost("host3") {
        ip "192.168.0.3"
        promisc "On"
      }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
      """

  @slow_process
  Scenario: Run "Repeater Hub" C example
    Given I run `trema run ../../objects/examples/repeater_hub/repeater_hub -c repeater_hub.conf -d`
     And wait until "repeater_hub" is up
    When I send 1 packet from host1 to host2
     And I run `trema show_stats host1 --tx`
     And I run `trema show_stats host2 --rx`
     And I run `trema show_stats host3 --rx`
    Then the output from "trema show_stats host1 --tx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
     And the output from "trema show_stats host2 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
     And the output from "trema show_stats host3 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"

  @slow_process
  Scenario: Run "Repeater Hub" Ruby example
    Given I run `trema run ../../src/examples/repeater_hub/repeater-hub.rb -c repeater_hub.conf -d`
     And wait until "RepeaterHub" is up
    When I send 1 packet from host1 to host2
     And I run `trema show_stats host1 --tx`
     And I run `trema show_stats host2 --rx`
     And I run `trema show_stats host3 --rx`
    Then the output from "trema show_stats host1 --tx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
     And the output from "trema show_stats host2 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
     And the output from "trema show_stats host3 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
