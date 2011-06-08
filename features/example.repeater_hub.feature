Feature: Control one openflow switch with repeater_hub controller

  As a Trema user
  I want to control one openflow switch using repeater_hub controller
  So that I can flood incoming packets to every other port


  Scenario: Run repeater hub
    When I try trema run "./objects/examples/repeater_hub/repeater_hub" with following configuration (backgrounded):
      """
      vswitch("repeater_hub") { datapath_id "0xabc" }

      vhost("host1") { promisc "On" }
      vhost("host2") { promisc "On" }
      vhost("host3") { promisc "On" }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
      """
      And wait until "repeater_hub" is up
      And I send 1 packet from host1 to host2
      And I try to run "./trema show_stats host1 --tx" (log = "host1.repeater_hub.log")
      And I try to run "./trema show_stats host2 --rx" (log = "host2.repeater_hub.log")
      And I try to run "./trema show_stats host3 --rx" (log = "host3.repeater_hub.log")
    Then the content of "host1.repeater_hub.log" and "host2.repeater_hub.log" should be identical
     And the content of "host1.repeater_hub.log" and "host3.repeater_hub.log" should be identical


  Scenario: Run repeater hub (Ruby)
    When I try trema run "./src/examples/repeater_hub/repeater-hub.rb" with following configuration (backgrounded):
      """
      vswitch("repeater_hub") { datapath_id "0xabc" }

      vhost("host1") { promisc "On" }
      vhost("host2") { promisc "On" }
      vhost("host3") { promisc "On" }

      link "repeater_hub", "host1"
      link "repeater_hub", "host2"
      link "repeater_hub", "host3"
      """
      And wait until "RepeaterHub" is up
      And I send 1 packet from host1 to host2
      And I try to run "./trema show_stats host1 --tx" (log = "host1.repeater-hub.rb.log")
      And I try to run "./trema show_stats host2 --rx" (log = "host2.repeater-hub.rb.log")
      And I try to run "./trema show_stats host3 --rx" (log = "host3.repeater-hub.rb.log")
    Then the content of "host1.repeater-hub.rb.log" and "host2.repeater-hub.rb.log" should be identical
     And the content of "host1.repeater-hub.rb.log" and "host3.repeater-hub.rb.log" should be identical
