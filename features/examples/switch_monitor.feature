Feature: "Switch Monitor" sample application

  In order to learn how to listen switch UP/DOWN event
  As a developer using Trema
  I want to execute "Switch Monitor" sample application

  Background:
    Given a file named "switch_monitor.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      vswitch { datapath_id 0x3 }
      """

  @slow_process
  Scenario: Run "Switch Monitor" C example
    Given I run `trema run ../../objects/examples/switch_monitor/switch_monitor -c switch_monitor.conf -d`
     And wait until "switch_monitor" is up
    When I run `trema kill 0x3`
     And *** sleep 1 ***
    Then the file "../../tmp/log/switch_monitor.log" should contain "Switch 0x3 is DOWN"
    When I run `trema up 0x3`
     And *** sleep 10 ***
    Then the file "../../tmp/log/switch_monitor.log" should contain "All switches = 0x1, 0x2, 0x3"

  @slow_process
  Scenario: Run "Switch Monitor" Ruby example
    Given I run `trema run ../../src/examples/switch_monitor/switch-monitor.rb -c switch_monitor.conf -d`
     And wait until "SwitchMonitor" is up
    When I run `trema kill 0x3`
     And *** sleep 1 ***
    Then the file "../../tmp/log/SwitchMonitor.log" should contain "Switch 0x3 is DOWN"
    When I run `trema up 0x3`
     And *** sleep 10 ***
    Then the file "../../tmp/log/SwitchMonitor.log" should contain "All switches = 0x1, 0x2, 0x3"
