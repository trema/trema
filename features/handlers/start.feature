Feature: start handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "hello.rb" with:
      """ruby
      class Hello < Trema::Controller
        def start(args)
          logger.info "args = [#{args.join(', ')}]"
        end
      end
      """

  @sudo
  Scenario: invoke start handler
    When I successfully run `trema run hello.rb -d`
    Then the file "Hello.log" should contain "args = []"

  @sudo
  Scenario: invoke start handler with args
    When I successfully run `trema run hello.rb -d -- arg0 arg1 arg2`
    Then the file "Hello.log" should contain "args = [arg0, arg1, arg2]"

  @sudo
  Scenario: invoke start handler (OpenFlow 1.3)
    When I successfully run `trema run hello.rb --openflow13 -d`
    Then the file "Hello.log" should contain "args = []"

  @sudo
  Scenario: invoke start handler with args (OpenFlow 1.3)
    When I successfully run `trema run hello.rb --openflow13 -d -- arg0 arg1 arg2`
    Then the file "Hello.log" should contain "args = [arg0, arg1, arg2]"

