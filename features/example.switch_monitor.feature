Feature: Hello trema sample application

  As a Trema user
  I want to list up OpenFlow switches in my network
  So that I can monitor my switches


  Scenario: switch-monitor
    Given I try trema run "./src/examples/switch_monitor/switch-monitor.rb" with following configuration (backgrounded):
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      vswitch { datapath_id 0x3 }
      """
      And *** sleep 2 ***
      And wait until "SwitchMonitor" is up
    When I try trema kill "0x3"
      And *** sleep 2 ***
    Then the log file "SwitchMonitor.log" should match:
      """
      Switch 0x3 is DOWN
      """
    When I try trema up "0x3"
      And *** sleep 10 ***
    Then the log file "SwitchMonitor.log" should match:
      """
      All switches = 0x1, 0x2, 0x3
      """


  Scenario: switch-monitor in C
    Given I try trema run "./objects/examples/switch_monitor/switch_monitor" with following configuration (backgrounded):
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      vswitch { datapath_id 0x3 }
      """
      And *** sleep 2 ***
      And wait until "switch_monitor" is up
    When I try trema kill "0x3"
      And *** sleep 2 ***
    Then the log file "switch_monitor.log" should match:
      """
      Switch 0x3 is DOWN
      """
    When I try trema up "0x3"
      And *** sleep 10 ***
    Then the log file "switch_monitor.log" should match:
      """
      All switches  = 0x1, 0x2, 0x3
      """
