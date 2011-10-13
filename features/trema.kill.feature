Feature: kill a trema process with `trema kill' command

  As a Trema user
  I want to kill a trema processes with `trema kill' command
  So that I can emulate failures on switch/host/link


  Scenario: trema kill
    Given I try trema run "./src/examples/switch_monitor/switch-monitor.rb" with following configuration (backgrounded):
      """
      vswitch { datapath_id "0x1" }
      vswitch { datapath_id "0x2" }
      vswitch { datapath_id "0x3" }
      """
      And wait until "SwitchMonitor" is up
    When I try to run "./trema kill 0x1"
      And *** sleep 1 ***
    Then vswitch 0x1 is terminated


  Scenario: trema help kill
    When I try to run "./trema help kill"
    Then the output should be:
      """
      Usage: ./trema kill NAME [OPTIONS ...]
          -h, --help
          -v, --verbose

      """
