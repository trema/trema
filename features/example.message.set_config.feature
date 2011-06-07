Feature: Send set config messages

  As a Trema user
  I want to send set config messages to openflow switches
  So that I can set configuration parameters of openflow switches


  Scenario: set config x 10
    When I try trema run "./objects/examples/openflow_message/set_config 10" with following configuration (backgrounded):
      """
      vswitch("set_config") { datapath_id "0xabc" }
      """
      And wait until "set_config" is up
      And I terminated all trema services
    Then the log file "openflowd.set_config.log" should include "received: set_config" x 11
