Feature: run trema application with `trema run' command

  As a Trema user
  I want to launch trema application with `trema run' command
  So that I don't have to mind dirty details of Trema

  Background:
    Given I terminated all trema services

  Scenario: trema run launches switch_manager and an application
    When I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      vswitch {
        datapath_id "0xabc"
      }
      """
      And wait until "dumper" is up
      And *** sleep 5 ***
    Then switch_manager is started
      And dumper is started

  Scenario: trema run launches switch_manager and an application with proper options
    When I try trema run "./objects/examples/learning_switch/learning_switch -i 0xabc" with following configuration (backgrounded, verbose):
      """
      vswitch {
        datapath_id "0xabc"
      }
      """
      And wait until "learning_switch" is up
      And *** sleep 5 ***
    Then "switch_manager" should be executed with option = "--daemonize --port=6633 -- port_status::learning_switch packet_in::learning_switch state_notify::learning_switch"
      And "learning_switch" should be executed with option = "--name learning_switch -i 0xabc"

  Scenario: switch_manager is killed when trema session is closed
    When I try trema run "/bin/true" with following configuration (backgrounded, verbose):
      """
      vswitch {
        datapath_id "0xabc"
      }
      """
      And wait until trema session is closed
    Then switch_manager should be killed

  Scenario: trema help run
    When I try to run "./trema help run"
    Then the output should be:
      """
      Usage: ./trema run [OPTIONS ...]
          -c, --conf FILE
          -d, --daemonize

          -h, --help
          -v, --verbose
      """
