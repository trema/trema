Feature: run command

  In order to launch trema applications
  As a developer using Trema
  I want to use `trema run` command

  Background:
    Given a file named "network.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """

  @slow_process
  Scenario: trema run launches switch_manager and an application
    When I run `trema run ../../objects/examples/dumper/dumper -c network.conf -d`
     And wait until "dumper" is up
    Then switch_manager is started
     And dumper is started

  @slow_process
  Scenario: trema run launches switch_manager and an application with proper options
    When I run `trema -v run ../../objects/examples/learning_switch/learning_switch -c network.conf -d`
     And wait until "learning_switch" is up
    Then the output should contain:
     """
     switch_manager --daemonize --port=6633 -- port_status::learning_switch packet_in::learning_switch state_notify::learning_switch vendor::learning_switch
     """
     And the output should contain:
     """
     learning_switch --name learning_switch -d
     """

  @slow_process
  Scenario: switch_manager is killed when trema session is closed
    When I run `trema -v run /bin/true -c network.conf`
    Then the output should contain "Shutting down switch_manager..."
