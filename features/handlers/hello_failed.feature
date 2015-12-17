Feature: hello_failed handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "hello_fails.rb" with:
      """ruby
      require 'version_mismatch'

      class HelloFails < Trema::Controller
        def hello_failed(message)
          logger.info 'Hello failed.'
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: invoke hello_failed handler
    Given I use OpenFlow 1.0
    And a file named "version_mismatch.rb" with:
      """ruby
      module Trema
        class Switch
          private

          # force trema to send Hello1.3 on startup.
          def exchange_hello_messages
            write Pio::OpenFlow13::Hello.new
            expect_receiving Hello
          end
        end
      end
      """
    When I trema run "hello_fails.rb" with the configuration "trema.conf"
    Then the file "HelloFails.log" should contain "Hello failed."

  @sudo
  Scenario: invoke hello_failed handler (OpenFlow 1.3)
    Given I use OpenFlow 1.3
    And a file named "version_mismatch.rb" with:
      """ruby
      module Trema
        class Switch
          private

          # force trema to send Hello1.0 on startup.
          def exchange_hello_messages
            write Pio::OpenFlow10::Hello.new
            expect_receiving Hello
          end
        end
      end
      """
    When I trema run "hello_fails.rb" with the configuration "trema.conf"
    Then the file "HelloFails.log" should contain "Hello failed."
