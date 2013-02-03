Feature: kill command

  In order to test event handling or just kill unwanted processes
  As a developer using Trema
  I want to execute "trema kill" command

  Background:
    Given a file named "switch_monitor.conf" with:
      """
      vswitch { datapath_id 0x1 }
      vswitch { datapath_id 0x2 }
      vswitch { datapath_id 0x3 }

      vhost "host1"
      vhost "host2"
      vhost "host3"

      link "0x1", "host1"
      link "0x2", "host2"
      link "0x3", "host3"
      """
    And I successfully run `trema run ../../src/examples/switch_monitor/switch-monitor.rb -c switch_monitor.conf -d`

  @slow_process
  Scenario: kill a switch
    When I run `trema kill 0x1`
    Then the vswitch "0x1" is terminated

  @slow_process
  Scenario: kill a host
    When I run `trema kill host1`
    Then the vhost "host1" is terminated

  @slow_process
  Scenario: kill hosts
    When I run `trema kill host1 host2`
    Then the vhost "host1" is terminated
     And the vhost "host2" is terminated

  @slow_process
  Scenario: kill a controller
    When I run `trema kill SwitchMonitor`
    Then the controller "SwitchMonitor" is terminated

  Scenario: no argument
    When I run `trema kill`
    Then the output should contain "name is required"

  Scenario: wrong name
    When I run `trema kill nosuchname`
    Then the output should contain "unknown name: nosuchname"
