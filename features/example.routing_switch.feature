Feature: control multiple openflow switchies using routing_switch

  As a Trema user
  I want to control multiple openflow switches using routing_switch application
  So that I can send and receive packets


  Scenario: One openflow switch, two servers
    When I try trema run "./objects/examples/routing_switch/routing_switch" with following configuration (backgrounded):
      """
      vswitch("routing_switch") { datapath_id "0xabc" }

      vhost("host1")
      vhost("host2")

      link "routing_switch", "host1"
      link "routing_switch", "host2"

      app { path "./objects/examples/topology/topology" }
      app { path "./objects/examples/topology/topology_discovery" }

      event :port_status => "topology", :packet_in => "filter", :state_notify => "topology"
      filter :lldp => "topology_discovery", :packet_in => "routing_switch"
      """
      And wait until "routing_switch" is up
      And *** sleep 10 ***
      And I try to run "./trema send_packets --source host1 --dest host2 --duration 10"
      And I try to run "./trema show_stats host1 --tx" (log = "tx.host1.log")
      And I try to run "./trema show_stats host2 --rx" (log = "rx.host2.log")
      And I terminated all trema services
    Then the content of "tx.host1.log" and "rx.host2.log" should be identical


  Scenario: One openflow switch, two servers
    When I try trema run "./objects/examples/routing_switch/routing_switch pathresolver topology" with following configuration (backgrounded):
      """
      vswitch("routing_switch1") { datapath_id "0x1" }
      vswitch("routing_switch2") { datapath_id "0x2" }
      vswitch("routing_switch3") { datapath_id "0x3" }
      vswitch("routing_switch4") { datapath_id "0x4" }

      vhost("host1")
      vhost("host2")
      vhost("host3")
      vhost("host4")

      link "routing_switch1", "host1"
      link "routing_switch2", "host2"
      link "routing_switch3", "host3"
      link "routing_switch4", "host4"
      link "routing_switch1", "routing_switch2"
      link "routing_switch1", "routing_switch3"
      link "routing_switch1", "routing_switch4"
      link "routing_switch2", "routing_switch3"
      link "routing_switch2", "routing_switch4"
      link "routing_switch3", "routing_switch4"

      app { path "./objects/examples/topology/topology" }
      app { path "./objects/examples/topology/topology_discovery" }

      event :port_status => "topology", :packet_in => "filter", :state_notify => "topology"
      filter :lldp => "topology_discovery", :packet_in => "routing_switch"
      """
      And wait until "routing_switch" is up
      And *** sleep 10 ***
      And I try to run "./trema send_packets --source host1 --dest host4 --duration 10"
      And I try to run "./trema show_stats host1 --tx" (log = "tx.host1.log")
      And I try to run "./trema show_stats host4 --rx" (log = "rx.host4.log")
      And I terminated all trema services
    Then the content of "tx.host1.log" and "rx.host4.log" should be identical


  Scenario: routing_switch --help
    When I try to run "./objects/examples/routing_switch/routing_switch --help"
    Then the output should be:
      """
      Switching HUB.
      Usage: routing_switch [OPTION]...

        -i, --idle_timeout=TIMEOUT  Idle timeout value of flow entry
        -n, --name=SERVICE_NAME     service name
        -t, --topology=SERVICE_NAME topology service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """


  Scenario: routing_switch -h
    When I try to run "./objects/examples/routing_switch/routing_switch -h"
    Then the output should be:
      """
      Switching HUB.
      Usage: routing_switch [OPTION]...

        -i, --idle_timeout=TIMEOUT  Idle timeout value of flow entry
        -n, --name=SERVICE_NAME     service name
        -t, --topology=SERVICE_NAME topology service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """
