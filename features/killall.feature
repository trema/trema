Feature: kill all trema processes with `trema killall' command

  As a Trema user
  I want to kill all heilos processes with `trema killall' command
  So that I can terminate trema session without investigating process table using ps command

  Scenario: trema killall
    Given I terminated all trema services
      And I try trema run with following configuration:
      """
      vswitch {
        datapath_id "0xabc"
      }

      app {
        path "./examples/dumper/dumper"
      }
      """
      And *** sleep 3 ***
    When I try to run "./trema killall"
    Then switch_manager is terminated
      And switch is terminated
      And ovs-openflowd is terminated
      And dumper is terminated

  Scenario: trema help killall
    When I try to run "./trema help killall"
    Then the output should be:
      """
      Usage: ./trema killall [OPTIONS ...]
          -h, --help
          -v, --verbose

      """
