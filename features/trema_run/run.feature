Feature: run
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: run controller_file
    Given a file named "hello.rb" with:
      """ruby
      class Hello < Trema::Controller
        def start(_args)
          logger.info 'Hello'
        end
      end
      """
    When I run `trema run hello.rb` interactively
    Then I stop the command if stdout contains:
      """
      Hello
      """

  @sudo
  Scenario: "No controller class is defined" error
    Given a file named "empty.rb" with:
      """
      """
    When I trema run "empty.rb"
    Then the exit status should not be 0
    And the stderr should contain:
      """
      No controller class is defined.
      """

  @sudo
  Scenario: SyntaxError
    Given a file named "invalid_ruby.rb" with:
      """
      Today is 13 March 2015
      """
    When I run `trema run invalid_ruby.rb`
    And the exit status should not be 0
    Then the output should contain "(SyntaxError)"

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
    When I run `trema run invalid_ruby.rb`
    Then the exit status should not be 0
    And the output should contain "uninitialized constant InvalidRuby::Foo (NameError)"

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
    When I run `trema run start_fail.rb`
    Then the exit status should not be 0
    And the output should contain "bang! (RuntimeError)"

  @sudo
  Scenario: cleanup on failure
    Given a file named "switch_ready_fail.rb" with:
      """ruby
      class SwitchReadyFail < Trema::Controller
        def switch_ready(_dpid)
          fail 'bang!'
        end
      end
      """
    When I run `trema -v run switch_ready_fail.rb -c trema.conf`
    Then virtual links should not exist 
    And the following files should not exist:
      | SwitchReadyFail.pid    |
      | vhost.host1.pid        |
      | vhost.host2.pid        |
