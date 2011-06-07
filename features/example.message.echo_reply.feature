Feature: Send echo reply messages

  As a Trema user
  I want to send echo reply messages to openflow switches
  Because I want to reply to echo requests from openflow switches


  Scenario: Send and echo reply x 10
    When I try trema run "./objects/examples/openflow_message/echo_reply 10" with following configuration (backgrounded):
      """
      vswitch("echo_reply") { datapath_id "0xabc" }
      """
      And wait until "echo_reply" is up
      And *** sleep 1 ***
    Then the log file "openflowd.echo_reply.log" should include "received: echo_reply" x 10
