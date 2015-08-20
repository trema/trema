Feature: hello_failed handler
  Background:
    Given a file named "trema.conf" with:
      """
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: invoke hello_failed handler
    Given a file named "hello_fails.rb" with:
      """
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
  Scenario: invoke hello_failed handler (OpenFlow 1.3)
    Given a file named "hello_fails.rb" with:
      """
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

      class HelloFails < Trema::Controller
        def hello_failed(message)
          logger.info 'Hello failed.'
        end
      end
      """
    When I successfully run `trema -v run hello_fails.rb -c trema.conf --openflow13 -d`
    And I run `sleep 5`
    Then the file "HelloFails.log" should contain "Hello failed."
