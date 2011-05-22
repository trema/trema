Feature: Send echo request messages

  As a Trema user
  I want to send echo request messages to openflow switches
  So that I can receive echo reply messages from openflow switches

  Background:
    Given I terminated all trema services

  Scenario: Send an echo request x 10
    When I try trema run "./objects/examples/openflow_message/echo_request 10" with following configuration (backgrounded):
      """
      vswitch {
        datapath_id "0xabc"
      }
      """
      And wait until "echo_request" is up
      And *** sleep 5 ***
      And I terminated all trema services
    Then the log file "./tmp/log/openflowd.0xabc.log" should include "received: echo_request" x 10
