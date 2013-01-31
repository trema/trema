Feature: killall command

  In order to cleanup the previous trema session
  As a developer using Trema
  I want to execute "trema killall" command

  @slow_process
  Scenario: trema killall
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
     And I successfully run `trema run ../../objects/examples/switch_monitor/switch_monitor -c switch_monitor.conf -d`
     And wait until "switch_monitor" is up
    When I run `trema killall`
    Then switch_manager is terminated
     And switch is terminated
     And phost is terminated
     And ovs-openflowd is terminated
     And switch_monitor is terminated
