Feature: "Hello Trema!" example

  The "Hello Trema!" example ([trema]/src/examples/hello_trema/) is one
  of the simplest OpenFlow controller implementation. The basic
  functionality of this controller is to establish a secure channel connection
  with an OpenFlow switch and output the "Hello [switch's dpid]!" message.

  This demonstrates a minimum template for Trema applications written in Ruby
  or C. Hence it's a good starting point to learn about Trema programming.

  Background:
    Given I cd to "../../src/examples/hello_trema/"

  @slow_process
  Scenario: Run the Ruby example
    When I run `trema run ./hello-trema.rb -c sample.conf` interactively
    Then the output should contain "Hello 0xabc!" within the timeout period

  @slow_process
  Scenario: Run the C example
    Given I compile "hello_trema.c" into "hello_trema"
    When I run `trema run ./hello_trema -c sample.conf` interactively
    Then the output should contain "Hello 0xabc!" within the timeout period
