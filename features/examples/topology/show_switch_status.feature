Feature: show_switch_status example.
  
  show_switch_status is a simple usage example of topology C API.
  show-switch-status.rb is a simple usage example of topology Ruby API.
  
  show_switch_status command will query for all the switch and port information 
  that the topology daemon hold and print them to standard output.

  Background: 
    Given I cd to "../../src/examples/topology/"

  @slow_process
  Scenario: [C API] Show switch and port information obtained from topology.
    Given I compile "show_switch_status.c" into "show_switch_status"
    Given a file named "show_switch_status.conf" with:
      """
      vswitch("topology1") { datapath_id "0xe0" }
      vhost ("host1") {
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }
      
      vhost ("host2") {
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }
      
      link "topology1", "host1"
      link "topology1", "host2"
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c show_switch_status.conf -d`
    And I run `trema run "../../../objects/topology/topology -d"`
    And *** sleep 4 ***
    When I run `trema run ./show_switch_status`
    Then the output should contain:
      """
      Switch status
        dpid : 0xe0, status : up
      """
    And the output should contain:
      """
      Port status
      """
    And the output should match /  dpid : 0xe0, port : 1\(.+\), status : up, external : (true|false)/
    And the output should match /  dpid : 0xe0, port : 2\(.+\), status : up, external : (true|false)/

  @slow_process
  Scenario: [Ruby API] Show switch and port information obtained from topology.
    Given a file named "show_switch_status.conf" with:
      """
      vswitch("topology1") { datapath_id "0xe0" }
      vhost ("host1") {
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }
      
      vhost ("host2") {
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }
      
      link "topology1", "host1"
      link "topology1", "host2"
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c show_switch_status.conf -d`
    And I run `trema run "../../../objects/topology/topology -d"`
    And *** sleep 4 ***
    When I run `trema run ./show-switch-status.rb`
    Then the output should contain:
      """
      Switch status
        dpid : 0xe0, status : up
      """
    And the output should contain:
      """
      Port status
      """
    And the output should match /  dpid : 0xe0, port : 1\(.+\), status : up, external : (true|false)/
    And the output should match /  dpid : 0xe0, port : 2\(.+\), status : up, external : (true|false)/
