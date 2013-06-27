Feature: reset_stats command

  In order to reset the stats of sent/received packets
  As a developer using Trema
  I want to execute "trema reset_stats" command

  Background:
    Given a file named "learning_switch.conf" with:
      """
      vswitch { datapath_id 0xabc }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "0xabc", "host1"
      link "0xabc", "host2"
      """
    And I run `trema run ../../src/examples/learning_switch/learning-switch.rb -c learning_switch.conf -d`
    And I run `trema send_packets --source host1 --dest host2`
    And *** sleep 1 ***

  @slow_process
  Scenario: reset_stats host1
    When I run `trema reset_stats host1`
     And I run `trema show_stats host1`
    Then the output should contain exactly "Sent packets:\n\nReceived packets:\n\n"

  @slow_process
  Scenario: reset_stats host1 host2
    When I run `trema reset_stats host1 host2`
     And I run `trema show_stats host1`
    Then the output should contain exactly "Sent packets:\n\nReceived packets:\n\n"

  @slow_process
  Scenario: reset_stats host1 host2
    When I run `trema reset_stats host1 host2`
     And I run `trema show_stats host2`
    Then the output should contain exactly "Sent packets:\n\nReceived packets:\n\n"

  @slow_process
  Scenario: no argument
    When I run `trema reset_stats`
     And I run `trema show_stats host1`
    Then the output should contain exactly "Sent packets:\n\nReceived packets:\n\n"

  @slow_process
  Scenario: no argument
    When I run `trema reset_stats`
     And I run `trema show_stats host2`
    Then the output should contain exactly "Sent packets:\n\nReceived packets:\n\n"

  @slow_process
  Scenario: wrong name
    When I run `trema reset_stats nosuchhost`
    Then the output should contain "unknown host: nosuchhost"
