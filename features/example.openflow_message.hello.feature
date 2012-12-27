Feature: Send hello messages

  In order to start conversations with switches
  As a Trema user
  I want to send hello messages to openflow switches

  @slow_process
  Scenario: Hello message in C
    Given a file named "hello.conf" with:
      """
      custom_switch("hello") { 
        datapath_id "0xabc" 
        path "./objects/examples/openflow_switch/hello_switch"
      }
      """
    When I run `trema run "../../objects/examples/openflow_message/hello 10" -c hello.conf -d`
      And wait until "hello" is up
      And I run `trema killall`
    Then the file "../../tmp/log/customswitch.hello.log" should contain:
      """
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      """

  @slow_process
  Scenario: Hello message in Ruby
    Given a file named "hello-r.conf" with:
      """
      custom_switch("hello-r") { 
        datapath_id "0xabc" 
        path "./objects/examples/openflow_switch/hello_switch"
      }
      """
    When I run `trema run "../../src/examples/openflow_message/hello.rb 10" -c hello-r.conf -d`
      And wait until "HelloController" is up
      And I run `trema killall`
    Then the file "../../tmp/log/customswitch.hello-r.log" should contain:
      """
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      received: OFPT_HELLO
      """
