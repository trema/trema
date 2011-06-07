Feature: Hello trema sample application

  As a Trema user
  I want to write "hello trema" application
  So that I can learn how to write minimum trema application


  Scenario: Hello trema in C
    When I try trema run "./objects/examples/hello_trema/hello_trema" with following configuration:
      """
      vswitch { datapath_id "0xabc" }
      """
    Then the output should include:
      """
      Hello 0xabc from ./objects/examples/hello_trema/hello_trema!
      """


  Scenario: Hello trema in Ruby
    When I try trema run "./src/examples/hello_trema/hello_trema.rb" with following configuration:
      """
      vswitch { datapath_id "0xabc" }
      """
    Then the output should include:
      """
      Hello 0xabc from ./src/examples/hello_trema/hello_trema.rb!
      """
