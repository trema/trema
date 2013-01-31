Feature: "Hello Trema!" sample application

  In order to learn how to write minimum Trema application
  As a developer using Trema
  I want to execute "Hello Trema" sample application

  Background:
    Given a file named "hello.conf" with:
      """
      vswitch { datapath_id "0xabc" }
      """

  @slow_process
  Scenario: Run "Hello Trema!" C example
    When I run `trema run ../../objects/examples/hello_trema/hello_trema -c hello.conf`
    Then the output should contain exactly "Hello 0xabc!\n"

  @slow_process
  Scenario: Run "Hello Trema!" Ruby example
    When I run `trema run ../../src/examples/hello_trema/hello-trema.rb -c hello.conf`
    Then the output should contain exactly "Hello 0xabc!\n"
