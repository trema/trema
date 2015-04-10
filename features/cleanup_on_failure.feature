Feature: trema run cleanup on failure
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }

      vhost('host1') { ip '192.168.0.1' }
      vhost('host2') { ip '192.168.0.2' }

      link '0xabc', 'host1'
      link '0xabc', 'host2'
      """

  Scenario: NameError in config file
    Given a file named "invalid_trema.conf" with:
      """
      Foo Bar Baz
      """
    And a file named "null_controller.rb" with:
      """
      class NullController < Trema::Controller; end
      """
    When I run `trema run null_controller.rb -c invalid_trema.conf`
    Then the output should contain "uninitialized constant Phut::Syntax::Baz (NameError)"
    And the exit status should not be 0
    And virtual links should not exist
    And a file named "NullController.pid" should not exist

  Scenario: SyntaxError in config file
    Given a file named "invalid_trema.conf" with:
      """
      Today is 19 June 2015
      """
    And a file named "null_controller.rb" with:
      """
      class NullController < Trema::Controller; end
      """
    When I run `trema run null_controller.rb -c invalid_trema.conf`
    Then the output should contain "(SyntaxError)"
    And the exit status should not be 0
    And virtual links should not exist
    And a file named "NullController.pid" should not exist

  Scenario: SyntaxError when loading controller code
    Given a file named "invalid_ruby.rb" with:
      """
      Today is 13 March 2015
      """
    When I run `trema run invalid_ruby.rb -c trema.conf`
    Then the output should contain "(SyntaxError)"
    And the exit status should not be 0
    And virtual links should not exist 
    And the following files should not exist:
      | vhost.host1.pid        |
      | vhost.host2.pid        |

  @sudo
  Scenario: NameError when loading controller code
    Given a file named "invalid_ruby.rb" with:
      """
      class InvalidRuby < Trema::Controller
        Foo
        Bar
        Baz
      end
      """
    When I run `trema run invalid_ruby.rb -c trema.conf`
    Then the output should contain "uninitialized constant InvalidRuby::Foo (NameError)"
    And the exit status should not be 0
    And virtual links should not exist 
    And the following files should not exist:
      | InvalidRuby.pid        |
      | vhost.host1.pid        |
      | vhost.host2.pid        |

  @sudo
  Scenario: RuntimeError in Controller#start
    Given a file named "start_fail.rb" with:
      """
      class StartFail < Trema::Controller
        def start(_args)
          fail 'bang!'
        end
      end
      """
    When I run `trema run start_fail.rb -c trema.conf`
    Then the output should contain "bang! (RuntimeError)"
    And the exit status should not be 0
    And virtual links should not exist 
    And the following files should not exist:
      | StartFail.pid          |
      | vhost.host1.pid        |
      | vhost.host2.pid        |

  @sudo
  Scenario: RuntimeError in handlers
    Given a file named "switch_ready_fail.rb" with:
      """
      class SwitchReadyFail < Trema::Controller
        def switch_ready(_dpid)
          fail 'bang!'
        end
      end
      """
    When I run `trema -v run switch_ready_fail.rb -c trema.conf`
    Then the output should contain "bang! (RuntimeError)"
    And the exit status should not be 0
    And virtual links should not exist 
    And the following files should not exist:
      | SwitchReadyFail.pid    |
      | vhost.host1.pid        |
      | vhost.host2.pid        |
