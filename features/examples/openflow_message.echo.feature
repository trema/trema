Feature: Send echo request and receive echo reply messages

  In order to hear heart beats from openflow switches
  As a Trema user
  I want to send echo request messages to openflow switches and receive echo replies

  @slow_process
  Scenario: Echo request and reply in C
    Given a file named "echo.conf" with:
      """
      custom_switch("echo") { 
        datapath_id "0xabc" 
        path "./objects/examples/openflow_switch/echo_switch"
      }
      """
    When I run `trema run "../../objects/examples/openflow_message/echo 10" -c echo.conf -d`
      And wait until "echo" is up
      And I run `trema killall`
    Then the file "../../tmp/log/customswitch.echo.log" should contain:
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
      And the file "../../tmp/log/echo.log" should match /received: OFPT_ECHO_REPLY/

  @slow_process
  Scenario: Echo request and reply in Ruby
    Given a file named "echo.ruby.conf" with:
      """
      custom_switch("echo.ruby") { 
        datapath_id "0xabc" 
        path "./objects/examples/openflow_switch/echo_switch"
      }
      """
    When I run `trema run "../../src/examples/openflow_message/echo.rb 0xabc, 10" -c echo.ruby.conf -d`
      And wait until "EchoController" is up
      And I run `trema killall`
    Then the file "../../tmp/log/customswitch.echo.ruby.log" should contain:
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
      # And the file "../../tmp/log/EchoController.log" should match /received: OFPT_ECHO_REPLY/
