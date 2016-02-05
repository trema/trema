Feature: -c (--conf) option

  -c (--conf) option specifies emulated network configuration

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "hello.rb" with:
      """ruby
      class Hello < Trema::Controller
        def switch_ready(dpid)
          logger.info format('Hello %s!', dpid.to_hex)
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: -c option
    When I successfully run `trema run hello.rb -c trema.conf -d`
    And sleep 5
    Then the file "Hello.log" should contain "Hello 0xabc!"

  @sudo
  Scenario: --conf option
    When I successfully run `trema run hello.rb --conf trema.conf -d`
    And sleep 5
    Then the file "Hello.log" should contain "Hello 0xabc!"

  @sudo
  Scenario: "No such file" error
    When I run `trema run hello.rb -c nosuchfile -d`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      No such file
      """

  @sudo
  Scenario: NameError
    Given a file named "invalid_trema.conf" with:
      """
      Foo Bar Baz
      """
    And a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
    When I run `trema run null_controller.rb -c invalid_trema.conf`
    Then the exit status should not be 0
    Then the output should match /uninitialized constant .*::Baz \(NameError\)/

  @sudo
  Scenario: SyntaxError
    Given a file named "invalid_trema.conf" with:
      """
      Today is 19 June 2015
      """
    And a file named "null_controller.rb" with:
      """ruby
      class NullController < Trema::Controller; end
      """
    When I run `trema run null_controller.rb -c invalid_trema.conf`
    Then the exit status should not be 0
    And the output should contain "(SyntaxError)"
