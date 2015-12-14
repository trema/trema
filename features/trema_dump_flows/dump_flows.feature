Feature: dump_flows
  @sudo
  Scenario: dump_flows
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "hello.rb" with:
      """ruby
      class Hello < Trema::Controller
        def start(_args)
          logger.info 'Hello'
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """
    And I trema run "hello.rb" with the configuration "trema.conf"
    And I successfully run `sleep 10`
    When I successfully run `trema dump_flows 0xabc -S.`
    Then the output should contain:
      """
      NXAST_FLOW reply
      """
