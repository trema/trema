Feature: control one openflow switch using learning_switch

  As a Trema user
  I want to control one openflow switch using learning_switch
  So that I can send and receive packets


  Scenario: Send and receive packets
    Given I try trema run "./objects/examples/learning_switch/learning_switch" with following configuration (backgrounded):
      """
      vswitch("learning") { datapath_id "0xabc" }

      vhost("host1")
      vhost("host2")

      link "learning", "host1"
      link "learning", "host2"
      """
      And wait until "learning_switch" is up
    When I try to run "./trema send_packets --source host1 --dest host2"
      And I try to run "./trema show_stats host1 --tx" (log = "host1.learning_switch.log")
      And I try to run "./trema show_stats host2 --rx" (log = "host2.learning_switch.log")
    Then the content of "host1.learning_switch.log" and "host2.learning_switch.log" should be identical
