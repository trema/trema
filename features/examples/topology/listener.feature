Feature: topology listener example
  
  [trema]/src/topology/listener.rb is a simple usage example of topology Ruby API.
  
  listener.rb shows a typical use case of topology Ruby API.
  1. Obtain the initial state of the topology by get_all_{switch,link.port}
     and build the initial datastructure.
  2. Keep local datastructure updated by topology event handlers.
  
  Note:
  * This controller includes Topology module, which implicitly starts 
    topology daemon, if it not already running.
  * This controller defines link_state_updated handler, which implicitly 
    enables the topolgy daemon's link discovery feature.  

  Background: 
    Given I cd to "../../src/examples/topology/"

  Scenario: 
    When I run `trema run ./listener.rb -c topology_fullmesh.conf -d`
    And wait until "topology" is up
    And *** sleep 10 ***
    Then the file "../../../tmp/log/TopologyListener.log" should match /graph "\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\d" \{/
    Then the file "../../../tmp/log/TopologyListener.log" should match /  "0xe1"$/
    Then the file "../../../tmp/log/TopologyListener.log" should match /  "0xe2"$/
    Then the file "../../../tmp/log/TopologyListener.log" should match /  "0xe3"$/
    Then the file "../../../tmp/log/TopologyListener.log" should match /  "0xe0"$/
    Then the file "../../../tmp/log/TopologyListener.log" should contain:
      """
        "0xe1" -- "0xe3"
      """
    Then the file "../../../tmp/log/TopologyListener.log" should contain:
      """
        "0xe0" -- "0xe1"
      """
    Then the file "../../../tmp/log/TopologyListener.log" should contain:
      """
        "0xe1" -- "0xe2"
      """
    Then the file "../../../tmp/log/TopologyListener.log" should contain:
      """
        "0xe0" -- "0xe3"
      """
    Then the file "../../../tmp/log/TopologyListener.log" should contain:
      """
        "0xe0" -- "0xe2"
      """
    Then the file "../../../tmp/log/TopologyListener.log" should contain:
      """
        "0xe2" -- "0xe3"
      """
    Then the file "../../../tmp/log/TopologyListener.log" should contain:
      """
      }
      """
