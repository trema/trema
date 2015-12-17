Feature: logging
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: controller, vhost and vswitch creates log files
    Given a file named "hello.rb" with:
      """
      class Hello < Trema::Controller
        def start(_args)
          logger.info 'Konnichi Wa'
        end
      end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      vhost { ip '192.168.0.1' }
      link '0xabc', '192.168.0.1'
      """
    When I successfully run `trema run hello.rb -c trema.conf -d`
    And sleep 5
    Then the following files should exist:
      | Hello.log              |
      | vhost.192.168.0.1.log  |
    And the file "Hello.log" should contain "Konnichi Wa"

  @sudo
  Scenario: aliasing vhost changes its log file name
    Given a file named "null_controller.rb" with:
      """
      class NullController < Trema::Controller; end
      """
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      # alias 192.168.0.1 as host1
      vhost('host1') { ip '192.168.0.1' }
      link '0xabc', 'host1'
      """
    When I successfully run `trema run null_controller.rb -c trema.conf -d`
    And sleep 5
    Then the following files should exist:
      | NullController.log     |
      | vhost.host1.log        |

  @sudo
  Scenario: run multiple controllers, and each have its own log file
    Given a file named "two_controllers.rb" with:
      """
      class ChildController < Trema::Controller
        def start(args)
          logger.info 'I am child controller'
        end
      end

      class ParentController < Trema::Controller
        def start(args)
          ChildController.new.start(args)
          logger.info 'I am parent controller'
        end
      end
      """
    When I successfully run `trema run two_controllers.rb -d`
    And sleep 3
    Then the following files should exist:
      | ParentController.log |
      | ChildController.log  |
    And the file "ParentController.log" should contain "I am parent controller"
    And the file "ChildController.log" should contain "I am child controller"
