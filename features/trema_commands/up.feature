Feature: up command

  In order to restart killed processes
  As a developer using Trema
  I want to execute "trema up" command

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
    And I run `trema kill 0x1 0x2 0x3`
    And I run `trema kill host1 host2 host3`

  @slow_process
  Scenario: up a switch
    When I run `trema up 0x1`
    Then the vswitch "0x1" is running

  @wip
  Scenario: up a host

