Feature: "List Switches" sample application

  In order to learn how to get the list of OpenFlow switches
  As a developer using Trema
  I want to execute "List Switches" sample application

  Background:
    Given a file named "list_switches.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      vswitch { datapath_id 0x3 }
      vswitch { datapath_id 0x4 }
      """

  @slow_process
  Scenario: Run "List Switches" C example
    Given I run `trema run ../../objects/examples/list_switches/list_switches -c list_switches.conf`
    Then the file "../../tmp/log/list_switches.log" should contain "switches = 0x1, 0x2, 0x3, 0x4"

  @slow_process
  Scenario: Run "List Switches" Ruby example
    Given I run `trema run ../../src/examples/list_switches/list-switches.rb -c list_switches.conf`
    Then the file "../../tmp/log/ListSwitches.log" should contain "switches = 0x1, 0x2, 0x3, 0x4"
