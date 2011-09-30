Feature: trema dump_flows command

  As a Trema user
  I want to dump the flow table of OpenFlow switch
  So that I can debug my Trema apps


  Scenario: run trema dump_flows
    When I try trema run "./objects/examples/repeater_hub/repeater_hub" with following configuration (backgrounded):
      """
      vswitch("repeater_hub") { datapath_id "0xabc" }

      vhost("host1")
      vhost("host2")
      vhost("host3")

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
      """
      And wait until "repeater_hub" is up
      And I try to run "./trema send_packets --source host1 --dest host2"
      And *** sleep 1 ***
      And I try to run "./trema dump_flows repeater_hub" (log = "dump_flows.log")
    Then "dump_flows.log" should contain some flow entries
