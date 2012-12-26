Feature: "Learning Switch" sample application

  In order to learn how to implement software L2 switch
  As a developer using Trema
  I want to execute "Learning Switch" sample application

  Background:
    Given a file named "learning_switch.conf" with:
      """
      vswitch("learning") { datapath_id "0xabc" }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "learning", "host1"
      link "learning", "host2"
      """

  @slow_process
  Scenario: Run "Learning Switch" C example
    Given I run `trema run ../../objects/examples/learning_switch/learning_switch -c learning_switch.conf -d`
     And wait until "learning_switch" is up
    When I send 1 packet from host1 to host2
     And I run `trema show_stats host1 --tx`
     And I run `trema show_stats host2 --rx`
    Then the output from "trema show_stats host1 --tx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
     And the output from "trema show_stats host2 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"

  @slow_process
  Scenario: Run "Learning Switch" Ruby example
    Given I run `trema run ../../src/examples/learning_switch/learning-switch.rb -c learning_switch.conf -d`
     And wait until "LearningSwitch" is up
    When I send 1 packet from host1 to host2
     And I run `trema show_stats host1 --tx`
     And I run `trema show_stats host2 --rx`
    Then the output from "trema show_stats host1 --tx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
     And the output from "trema show_stats host2 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
