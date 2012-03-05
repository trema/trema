Feature: kill all trema processes with `trema killall' command

  As a Trema user
  I want to kill all trema processes with `trema killall' command
  So that I can close trema sessions cleanly


  Scenario: trema killall
    Given I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      vswitch { datapath_id "0xabc" }
      """
      And wait until "dumper" is up
    When I try to run "./trema killall"
      And *** sleep 1 ***
    Then switch_manager is terminated
      And switch is terminated
      And ovs-openflowd is terminated
      And dumper is terminated


  Scenario: trema help killall
    When I try to run "./trema help killall"
    Then the output should be:
      """
      Usage: trema killall [OPTIONS ...]
          -h, --help
          -v, --verbose

      """
