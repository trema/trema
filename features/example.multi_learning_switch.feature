Feature: "Multi Learning Switch" sample application

  In order to learn how to implement software L2 switch (multiple switch support)
  As a developer using Trema
  I want to execute "Multi Learning Switch" sample application

  Background:
    Given a file named "multi_learning_switch.conf" with:
      """
      vswitch("multi_learning1") { datapath_id "0x1" }
      vswitch("multi_learning2") { datapath_id "0x2" }
      vswitch("multi_learning3") { datapath_id "0x3" }
      vswitch("multi_learning4") { datapath_id "0x4" }

      vhost("host1")
      vhost("host2")
      vhost("host3")
      vhost("host4")

      link "multi_learning1", "host1"
      link "multi_learning2", "host2"
      link "multi_learning3", "host3"
      link "multi_learning4", "host4"
      link "multi_learning1", "multi_learning2"
      link "multi_learning2", "multi_learning3"
      link "multi_learning3", "multi_learning4"
      """

  @slow_process
  Scenario: Run "Multi Learning Switch" C example
    Given I run `trema run ../../objects/examples/multi_learning_switch/multi_learning_switch -c multi_learning_switch.conf -d`
     And wait until "multi_learning_switch" is up
    When I send 2 packets from host1 to host2
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     0 |     0 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     0 |     2 |     0 |     0 |
    When I send 3 packets from host3 to host4
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     3 |     0 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     0 |     2 |     0 |     3 |
    When I send 2 packets from host4 to host1
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     3 |     2 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     0 |     3 |
    When I send 4 packets from host2 to host3
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     4 |     3 |     2 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     4 |     3 |
    When I send 1 packets from host1 to host4
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     3 |     4 |     3 |     2 |
     And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     4 |     4 |

  @slow_process
  Scenario: Run "Multi Learning Switch" Ruby example
    Given I run `trema run ../../src/examples/multi_learning_switch/multi-learning-switch.rb -c multi_learning_switch.conf -d`
     And wait until "MultiLearningSwitch" is up
    When I send 2 packets from host1 to host2
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     0 |     0 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     0 |     2 |     0 |     0 |
    When I send 3 packets from host3 to host4
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     3 |     0 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     0 |     2 |     0 |     3 |
    When I send 2 packets from host4 to host1
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     3 |     2 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     0 |     3 |
    When I send 4 packets from host2 to host3
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     4 |     3 |     2 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     4 |     3 |
    When I send 1 packets from host1 to host4
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     3 |     4 |     3 |     2 |
     And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     4 |     4 |
