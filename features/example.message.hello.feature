Feature: Send hello messages

  As a Trema user
  I want to send hello messages to openflow switches
  So that I can start transactions with switches


  Scenario: Hello trema
    When I try trema run "./objects/examples/openflow_message/hello 10" with following configuration (backgrounded):
      """
      custom_switch("hello") { 
        datapath_id "0xabc" 
        path "./objects/examples/openflow_switch/hello_switch"
      }
      """
      And wait until "hello" is up
      And I terminated all trema services
    Then the log file "customswitch.hello.log" should include "received: OFPT_HELLO" x 11


  @wip
  Scenario: Hello trema in Ruby
    When I try trema run "./src/examples/openflow_message/hello.rb 0xabc, 10" with following configuration (backgrounded):
      """
      custom_switch("hello-r") {
        datapath_id "0xabc"
        path "./objects/examples/openflow_switch/hello_switch"
      }
      """
      And wait until "HelloController" is up
      And I terminated all trema services
    Then the log file "customswitch.hello-r.log" should include "received: OFPT_HELLO" x 11
