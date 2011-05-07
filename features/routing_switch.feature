Feature: control multiple openflow switchies using routing_switch

  As a Trema user
  I want to control multiple openflow switchies using routing_switch application
  So that I can send and receive packets

  Background:
    Given I terminated all trema services

  Scenario: One openflow switch, two servers
    When I try trema run "./objects/examples/routing_switch/routing_switch" with following configuration (backgrounded):
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
        path "./objects/examples/topology/topology"
      }

      app { 
        path "./objects/examples/topology/topology_discovery"
      }

      event :port_status => "topology", :packet_in => "filter", :state_notify => "topology"
      filter :lldp => "topology_discovery", :packet_in => "routing_switch"
      """
      And wait until "routing_switch" is up
      And *** sleep 10 ***
      And I try to run "./trema send_packets --source 192.168.0.1 --dest 192.168.0.2 --duration 10"
      And I try to run "./trema show_stats 192.168.0.1 --tx" (log = "tx.log")
      And I try to run "./trema show_stats 192.168.0.2 --rx" (log = "rx.log")
      And I terminated all trema services
    Then the content of "tx.log" and "rx.log" should be identical


  Scenario: One openflow switch, two servers
    When I try trema run "./objects/examples/routing_switch/routing_switch pathresolver topology" with following configuration (backgrounded):
      """
      vswitch {
        datapath_id "0xe0"
      }

      vswitch {
        datapath_id "0xe1"
      }

      vswitch {
        datapath_id "0xe2"
      }

      vswitch {
        datapath_id "0xe3"
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

      vhost {
        ip "192.168.0.3"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:03"
      }

      vhost {
        ip "192.168.0.4"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:04"
      }

      link "0xe0", "192.168.0.1"
      link "0xe1", "192.168.0.2"
      link "0xe2", "192.168.0.3"
      link "0xe3", "192.168.0.4"
      link "0xe0", "0xe1"
      link "0xe0", "0xe2"
      link "0xe0", "0xe3"
      link "0xe1", "0xe2"
      link "0xe1", "0xe3"
      link "0xe2", "0xe3"

      app {
        path "./objects/examples/topology/topology"
      }

      app {
        path "./objects/examples/topology/topology_discovery"
      }

      event :port_status => "topology", :packet_in => "filter", :state_notify => "topology"
      filter :lldp => "topology_discovery", :packet_in => "routing_switch"
      """
      And wait until "routing_switch" is up
      And *** sleep 20 ***
      And I try to run "./trema send_packets --source 192.168.0.1 --dest 192.168.0.4 --duration 10"
      And I try to run "./trema show_stats 192.168.0.1 --tx" (log = "tx.log")
      And I try to run "./trema show_stats 192.168.0.4 --rx" (log = "rx.log")
      And I terminated all trema services
    Then the content of "tx.log" and "rx.log" should be identical
