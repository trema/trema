Feature: Send echo reply messages

  As a Trema user
  I want to send echo reply messages to openflow switches
  Because I want to reply echo requests from openflow switches

  Background:
    Given I terminated all trema services

  Scenario: Send and echo reply x 10
    When I try trema run "./objects/examples/openflow_message/echo_reply 10" with following configuration:
      """
      vswitch {
        datapath_id "0xabc"
      }
      """
      And wait until "echo_reply" is up
      And I terminated all trema services
    Then the log file "./tmp/log/openflowd.0xabc.log" should include "received: echo_reply" x 10
