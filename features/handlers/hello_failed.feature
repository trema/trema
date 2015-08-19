Feature: hello_failed handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: hello_failed handler invoked
    Given a file named "hello_fails.rb" with:
      """
      module Trema
        class Switch
          private

          def exchange_hello_messages
            write Pio::OpenFlow13::Hello.new
            expect_receiving Hello
          end
        end
      end

      class HelloFails < Trema::Controller
        def hello_failed(message)
          logger.info 'Hello failed.'
        end
      end
      """
    When I successfully run `trema -v run hello_fails.rb -c trema.conf -d`
    And I run `sleep 5`
    Then the file "HelloFails.log" should contain "Hello failed."

  @sudo
  Scenario: hello_failed handler invoked (OpenFlow 1.3)
    Given a file named "hello_fails.rb" with:
      """
      module Trema
        class Switch
          private

          def exchange_hello_messages
            write Pio::OpenFlow10::Hello.new
            expect_receiving Hello
          end
        end
      end

      class HelloFails < Trema::Controller
        def hello_failed(message)
          logger.info 'Hello failed.'
        end
      end
      """
    When I successfully run `trema -v run hello_fails.rb -c trema.conf --openflow13 -d`
    And I run `sleep 5`
    Then the file "HelloFails.log" should contain "Hello failed."
