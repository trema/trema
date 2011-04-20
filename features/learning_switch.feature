Feature: control one openflow switch using learning_switch

  As a Trema user
  I want to control one openflow switch using learning_switch
  So that I can send and receive packets

  Background:
    Given I terminated all trema services

  Scenario: One openflow switch, two servers
    When I try trema run with following configuration:
      """
      vswitch {
        datapath_id "0xe0"
      }

      vhost {
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }

      vhost {
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }

      link "0xe0", "192.168.0.1"
      link "0xe0", "192.168.0.2"

      app {
        path "./objects/examples/learning_switch/learning_switch"
        options "-i", "0xe0"
      }
      """
      And *** sleep 10 ***
      And I try to run "./trema send_packets --source 192.168.0.1 --dest 192.168.0.2"
      And I try to run "./trema show_stats 192.168.0.1 --tx > ./tmp/log/tx.log"
      And I try to run "./trema show_stats 192.168.0.2 --rx > ./tmp/log/rx.log"
      And I terminated all trema services
    Then the content of "./tmp/log/tx.log" and "./tmp/log/rx.log" should be identical
