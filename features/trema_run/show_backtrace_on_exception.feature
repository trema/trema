Feature: Show backtrace on exception
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: "No controller class is defined" error
    Given an empty file named "empty.rb"
    When I trema run "empty.rb" interactively
    Then the exit status should not be 0
    And the stderr should contain:
      """
      Trema::NoControllerDefined: empty.rb: No controller class is defined
      """
    And the file "trema.log" should contain:
      """
      Trema::NoControllerDefined: empty.rb: No controller class is defined
      """

  @sudo
  Scenario: SyntaxError
    Given a file named "invalid_ruby.rb" with:
      """
      Today is 13 March 2015
      """
    When I trema run "invalid_ruby.rb" interactively
    And the exit status should not be 0
    Then the stderr should contain:
      """
      SyntaxError: invalid_ruby.rb:1: syntax error, unexpected tCONSTANT, expecting end-of-input
      Today is 13 March 2015
                       ^
      """
    And the file "trema.log" should contain:
      """
      SyntaxError: invalid_ruby.rb:1: syntax error, unexpected tCONSTANT, expecting end-of-input
      Today is 13 March 2015
                       ^
      """

  @sudo
  Scenario: NameError
    Given a file named "invalid_ruby.rb" with:
      """ruby
      class InvalidRuby < Trema::Controller
        Foo
        Bar
        Baz
      end
      """
    When I trema run "invalid_ruby.rb" interactively
    Then the exit status should not be 0
    And the stderr should contain:
      """
      NameError: uninitialized constant InvalidRuby::Foo
      """
    And the stderr should contain:
      """
      invalid_ruby.rb:2:in `<class:InvalidRuby>'
      """
    And the file "trema.log" should contain:
      """
      NameError: uninitialized constant InvalidRuby::Foo
      """
    And the file "trema.log" should contain:
      """
      invalid_ruby.rb:2:in `<class:InvalidRuby>'
      """

  @sudo
  Scenario: RuntimeError
    Given a file named "start_fail.rb" with:
      """ruby
      class StartFail < Trema::Controller
        def start(_args)
          fail 'bang!'
        end
      end
      """
    When I trema run "start_fail.rb" interactively
    Then the exit status should not be 0
    And the stderr should contain:
      """
      RuntimeError: bang!
      """
    And the stderr should contain:
      """
      start_fail.rb:3:in `start'
      """
    And the file "StartFail.log" should contain:
      """
      RuntimeError: bang!
      """
    And the file "StartFail.log" should contain:
      """
      start_fail.rb:3:in `start'
      """
