Feature: -c (--conf) option

  -c (--conf) option specifies emulated network configuration

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And I use a fixture named "event_logger"
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: -c option
    When I run `trema run event_logger.rb -c trema.conf` interactively
    And I stop the command if stderr contains:
      """
      EventLogger#switch_ready (dpid = 0xabc)
      """
    Then the file "EventLogger.log" should contain:
      """
      EventLogger#switch_ready (dpid = 0xabc)
      """

  @sudo
  Scenario: --conf option
    When I run `trema run event_logger.rb --conf trema.conf` interactively
    And I stop the command if stderr contains:
      """
      EventLogger#switch_ready (dpid = 0xabc)
      """
    Then the file "EventLogger.log" should contain:
      """
      EventLogger#switch_ready (dpid = 0xabc)
      """

  @sudo
  Scenario: "No such file" error
    When I run `trema run event_logger.rb -c nosuchfile`
    Then the exit status should not be 0
    And the stderr should contain:
      """
      Errno::ENOENT: No such file or directory @ rb_sysopen - nosuchfile
      """

  @sudo
  Scenario: NameError
    Given a file named "invalid_trema.conf" with:
      """
      Foo Bar Baz
      """
    When I run `trema run event_logger.rb -c invalid_trema.conf`
    Then the exit status should not be 0
    Then the output should match:
      """
      NameError: uninitialized constant .*::Baz
      """

  @sudo
  Scenario: SyntaxError
    Given a file named "invalid_trema.conf" with:
      """
      Today is 19 June 2015
      """
    When I run `trema run event_logger.rb -c invalid_trema.conf`
    Then the exit status should not be 0
    And the output should contain:
      """
      SyntaxError: invalid_trema.conf:1: syntax error, unexpected tCONSTANT, expecting end-of-input
      Today is 19 June 2015
                      ^
      """
