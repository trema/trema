Feature: Log file naming
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: controller and vhost creates its log file
    Given I use a fixture named "event_logger"
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      vhost { ip '192.168.0.1' }
      link '0xabc', '192.168.0.1'
      """
    When I run `trema run event_logger.rb -c trema.conf` interactively
    And I stop the command if stderr contains:
      """
      EventLogger#start (args = [])
      """
    Then the following files should exist:
      | EventLogger.log       |
      | vhost.192.168.0.1.log |
    And the file "EventLogger.log" should contain:
      """
      EventLogger#start (args = [])
      """

  @sudo
  Scenario: aliasing vhost changes its log file name
    Given I use a fixture named "event_logger"
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      # alias 192.168.0.1 as host1
      vhost('host1') { ip '192.168.0.1' }
      link '0xabc', 'host1'
      """
    When I run `trema run event_logger.rb -c trema.conf` interactively
    And I stop the command if stderr contains:
      """
      EventLogger#start (args = [])
      """
    Then the following files should exist:
      | EventLogger.log |
      | vhost.host1.log |

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
          logger.info 'I am parent controller'
          ChildController.new.start(args)
        end
      end
      """
    When I run `trema run two_controllers.rb` interactively
    And I stop the command if stderr contains:
      """
      I am parent controller
      """
    Then the following files should exist:
      | ParentController.log |
      | ChildController.log  |
    And the file "ParentController.log" should contain "I am parent controller"
    And the file "ChildController.log" should contain "I am child controller"
