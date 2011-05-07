Feature: control one openflow switch using learning_switch

  As a Trema user
  I want to control one openflow switch using learning_switch
  So that I can send and receive packets

  Background:
    Given I terminated all trema services

  Scenario: One openflow switch, two servers
    When I try trema run "./objects/examples/learning_switch/learning_switch -i 0xabc" with following configuration (backgrounded):
      """
      vswitch("lsw") {
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

      link "lsw", "host1"
      link "lsw", "host2"
      """
      And wait until "learning_switch" is up
      And I try to run "./trema send_packets --source host1 --dest host2"
      And I try to run "./trema show_stats host1 --tx > ./tmp/log/host1.learning_switch.log"
      And I try to run "./trema show_stats host2 --rx > ./tmp/log/host2.learning_switch.log"
      And I terminated all trema services
    Then the content of "./tmp/log/host1.learning_switch.log" and "./tmp/log/host2.learning_switch.log" should be identical
