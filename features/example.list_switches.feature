Feature: Get the list of openflow switches with list_switches

  As a Trema user
  I want to get the list of openflow switches
  So that I can check how many switches are connected to my controller


  Scenario: Get the list of openflow switches
    Given I try trema run "./objects/examples/list_switches/list_switches" with following configuration (backgrounded):
      """
      vswitch { datapath_id "0x1" }
      vswitch { datapath_id "0x2" }
      vswitch { datapath_id "0x3" }
      vswitch { datapath_id "0x4" }
      """
      And wait until "list_switches" is up
      And *** sleep 2 ***
      And I terminated all trema services
    Then the output should include:
      """
      switches = 0x1, 0x2, 0x3, 0x4
      """

  Scenario: Get the list of openflow switches (in Ruby)
    Given I try trema run "./src/examples/list_switches/list-switches.rb" with following configuration (backgrounded):
      """
      vswitch { datapath_id "0x1" }
      vswitch { datapath_id "0x2" }
      vswitch { datapath_id "0x3" }
      vswitch { datapath_id "0x4" }
      """
      And wait until "ListSwitches" is up
      And *** sleep 2 ***
      And I terminated all trema services
    Then the output should include:
      """
      switches = 0x1, 0x2, 0x3, 0x4
      """
