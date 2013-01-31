Feature: "Switch Info" sample application

  In order to learn how to acquire switch spec
  As a developer using Trema
  I want to execute "Switch Info" sample application

  @slow_process
  Scenario: Run "Switch Info" C example
    Given a file named "switch_info.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """
     And I run `trema run ../../objects/examples/switch_info/switch_info -c switch_info.conf -d`
     And *** sleep 2 ***
    Then the file "../../tmp/log/switch_info.log" should contain "datapath_id: 0xabc"
     And the file "../../tmp/log/switch_info.log" should contain "#ports: 1"

  @slow_process
  Scenario: Run "Switch Info" C example with two switches
   Given a file named "switch_info.conf" with:
      """
      vswitch { datapath_id 0xabc }
      vswitch { datapath_id 0xdef }
      link "0xabc", "0xdef"
      """
     And I run `trema run ../../objects/examples/switch_info/switch_info -c switch_info.conf -d`
     And *** sleep 2 ***
    Then the file "../../tmp/log/switch_info.log" should contain "#ports: 2"

  @slow_process
  Scenario: Run "Switch Info" Ruby example
    Given a file named "switch_info.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """
     And I run `trema run ../../src/examples/switch_info/switch-info.rb -c switch_info.conf -d`
     And *** sleep 2 ***
    Then the file "../../tmp/log/SwitchInfo.log" should contain "datapath_id: 0xabc"
     And the file "../../tmp/log/SwitchInfo.log" should contain "#ports: 1"

  @slow_process
  Scenario: Run "Switch Info" Ruby example with two switches
   Given a file named "switch_info.conf" with:
      """
      vswitch { datapath_id 0xabc }
      vswitch { datapath_id 0xdef }
      link "0xabc", "0xdef"
      """
     And I run `trema run ../../src/examples/switch_info/switch-info.rb -c switch_info.conf -d`
     And *** sleep 2 ***
    Then the file "../../tmp/log/SwitchInfo.log" should contain "#ports: 2"
