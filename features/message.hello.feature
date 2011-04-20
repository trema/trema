Feature: Send hello messages

  As a Trema user
  I want to send hello messages to openflow switches
  So that I can start transactions with switches

  Background:
    Given I terminated all trema services

  Scenario: Hello trema
    When I try trema run "./objects/examples/openflow_message/hello 10" with following configuration:
      """
      vswitch {
        datapath_id "0xabc"
      }
      """
    Then the log file "./tmp/log/openflowd.0xabc.log" should include "received: hello" x 11
