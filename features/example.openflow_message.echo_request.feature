Feature: Send echo request messages

  In order to hear heart beats from openflow switches
  As a Trema user
  I want to send echo request messages to openflow switches

  @slow_process
  Scenario: Echo request in C
    Given a file named "echo.conf" with:
      """
      custom_switch("echo_request") { 
        datapath_id "0xabc" 
        path "./objects/examples/openflow_switch/echo_switch"
      }
      """
    When I run `trema run "../../objects/examples/openflow_message/echo_request 10" -c echo.conf -d`
      And wait until "echo_request" is up
      And I run `trema killall`
    Then the file "../../tmp/log/customswitch.echo_request.log" should contain:
      """
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      """
      And the file "../../tmp/log/echo_request.log" should match /received: OFPT_ECHO_REPLY/

  @slow_process
  Scenario: Echo request in Ruby
    Given a file named "echo.ruby.conf" with:
      """
      custom_switch("echo_request.ruby") { 
        datapath_id "0xabc" 
        path "./objects/examples/openflow_switch/echo_switch"
      }
      """
    When I run `trema run "../../src/examples/openflow_message/echo-request.rb 0xabc, 10" -c echo.ruby.conf -d`
      And wait until "EchoRequestController" is up
      And I run `trema killall`
    Then the file "../../tmp/log/customswitch.echo_request.ruby.log" should contain:
      """
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      received: OFPT_ECHO_REQUEST
      """
      # And the file "../../tmp/log/EchoRequestController.log" should match /received: OFPT_ECHO_REPLY/
