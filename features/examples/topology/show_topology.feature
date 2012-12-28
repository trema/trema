Feature: show_topology example.
  
  show_topology is a simple usage example of topology C API.
  show-topology.rb is a simple usage example of topology Ruby API.
  
  show_topology command will query for all the link information that 
  the topology daemon hold and print them in trema network DSL style.
  
  Topology daemon's link discovery feature must be enabled prior to running 
  show_topology command in order to obtain non-empty result.
  
  Topology daemon's link discovery feature can be enabled 
  by specifing "--always_run_discovery" option when starting topology daemon,
  or by calling enable_topology_discovery() API from client application.  
  

  Background: 
    Given I cd to "../../src/examples/topology/"

  @slow_process
  Scenario: [C API] Show discovered link topology. (Directly start topology daemon)
    Given I compile "show_topology.c" into "show_topology"
    Given I compile "enable_discovery.c" into "enable_discovery"
    Given a file named "show_topology.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      vswitch("topology2") { datapath_id "0x2" }
      vswitch("topology3") { datapath_id "0x3" }
      vswitch("topology4") { datapath_id "0x4" }
      
      link "topology1", "topology2"
      link "topology1", "topology3"
      link "topology1", "topology4"
      link "topology2", "topology3"
      link "topology2", "topology4"
      link "topology3", "topology4"
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c show_topology.conf -d`
    And I run `trema run "../../../objects/topology/topology -d --always_run_discovery"`
    And *** sleep 4 ***
    When I run `trema run ./show_topology`
    Then the output should contain:
      """
      vswitch {
        datapath_id "0x2"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x3"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x1"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x4"
      }
      """
    And the output should contain:
      """
      link "0x2", "0x1"
      """
    And the output should contain:
      """
      link "0x3", "0x2"
      """
    And the output should contain:
      """
      link "0x3", "0x1"
      """
    And the output should contain:
      """
      link "0x4", "0x2"
      """
    And the output should contain:
      """
      link "0x4", "0x3"
      """
    And the output should contain:
      """
      link "0x4", "0x1"
      """

  @slow_process
  Scenario: [C API] Show discovered link topology. (Enable link discovery by API)
    Given I compile "show_topology.c" into "show_topology"
    Given I compile "enable_discovery.c" into "enable_discovery"
    Given a file named "show_topology.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      vswitch("topology2") { datapath_id "0x2" }
      vswitch("topology3") { datapath_id "0x3" }
      vswitch("topology4") { datapath_id "0x4" }
      
      link "topology1", "topology2"
      link "topology1", "topology3"
      link "topology1", "topology4"
      link "topology2", "topology3"
      link "topology2", "topology4"
      link "topology3", "topology4"
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c show_topology.conf -d`
    And I run `trema run "./enable_discovery"`
    And *** sleep 4 ***
    When I run `trema run ./show_topology`
    Then the output should contain:
      """
      vswitch {
        datapath_id "0x2"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x3"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x1"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x4"
      }
      """
    And the output should contain:
      """
      link "0x2", "0x1"
      """
    And the output should contain:
      """
      link "0x3", "0x2"
      """
    And the output should contain:
      """
      link "0x3", "0x1"
      """
    And the output should contain:
      """
      link "0x4", "0x2"
      """
    And the output should contain:
      """
      link "0x4", "0x3"
      """
    And the output should contain:
      """
      link "0x4", "0x1"
      """

  @slow_process
  Scenario: [Ruby API] Show discovered link topology. (Directly start topology daemon)
    Given a file named "show_topology.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      vswitch("topology2") { datapath_id "0x2" }
      vswitch("topology3") { datapath_id "0x3" }
      vswitch("topology4") { datapath_id "0x4" }
      
      link "topology1", "topology2"
      link "topology1", "topology3"
      link "topology1", "topology4"
      link "topology2", "topology3"
      link "topology2", "topology4"
      link "topology3", "topology4"
      """
    And I run `trema run ../repeater_hub/repeater-hub.rb -c show_topology.conf -d`
    And I run `trema run "../../../objects/topology/topology -d --always_run_discovery"`
    And *** sleep 4 ***
    When I run `trema run ./show-topology.rb`
    Then the output should contain:
      """
      vswitch {
        datapath_id "0x2"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x3"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x1"
      }
      """
    And the output should contain:
      """
      vswitch {
        datapath_id "0x4"
      }
      """
    And the output should contain:
      """
      link "0x2", "0x1"
      """
    And the output should contain:
      """
      link "0x3", "0x2"
      """
    And the output should contain:
      """
      link "0x3", "0x1"
      """
    And the output should contain:
      """
      link "0x4", "0x2"
      """
    And the output should contain:
      """
      link "0x4", "0x3"
      """
    And the output should contain:
      """
      link "0x4", "0x1"
      """

