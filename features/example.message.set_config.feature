Feature: Send set config messages

  As a Trema user
  I want to send set config messages to openflow switches
  So that I can set configuration parameters of openflow switches

  @wip
  Scenario: set config x 10
    When I try trema run "./objects/examples/openflow_message/set_config 10" with following configuration (backgrounded):
      """
      vswitch("set_config") { datapath_id "0xabc" }
      """
      And wait until "set_config" is up
      And *** sleep 2 ***
      And I run `trema killall`
    Then the log file "openflowd.set_config.log" should include "received: OFPT_SET_CONFIG" x 11

  @wip
  Scenario: set config x 10 in Ruby
    When I try trema run "./src/examples/openflow_message/set-config.rb 0xabc, 10" with following configuration (backgrounded):
      """
      vswitch("set-config") { datapath_id "0xabc" }
      """
      And wait until "SetConfigController" is up
      And *** sleep 2 ***
      And I run `trema killall`
    Then the log file "openflowd.set-config.log" should include "received: OFPT_SET_CONFIG" x 11
