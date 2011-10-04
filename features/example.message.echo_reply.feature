Feature: Send echo reply messages

  As a Trema user
  I want to send echo reply messages to openflow switches
  Because I want to reply to echo requests from openflow switches


  Scenario: Send echo reply x 10
    When I try trema run "./objects/examples/openflow_message/echo_reply 10" with following configuration (backgrounded):
      """
      vswitch("echo_reply") { datapath_id "0xabc" }
      """
      And wait until "echo_reply" is up
      And *** sleep 2 ***
    Then the log file "openflowd.echo_reply.log" should include "received: OFPT_ECHO_REPLY" x 10


   Scenario: Send echo reply x 10 in Ruby
    When I try trema run "./src/examples/openflow_message/echo-reply.rb 0xabc, 10" with following configuration (backgrounded):
      """
      vswitch("echo-reply") { datapath_id "0xabc" }
      """
      And wait until "EchoReplyController" is up
      And *** sleep 2 ***
    Then the log file "openflowd.echo-reply.log" should include "received: OFPT_ECHO_REPLY" x 10

