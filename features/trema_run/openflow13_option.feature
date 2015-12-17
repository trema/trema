Feature: --openflow13 option

  Use --openflow13 option to enable OpenFlow 1.3

  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "openflow_version.rb" with:
      """ruby
      class OpenflowVersion < Trema::Controller
        def switch_ready(dpid)
          send_message dpid, Echo::Request.new
        end

        def echo_reply(dpid, message)
          logger.info "ofp_version = #{message.ofp_version}"
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      """

  @sudo
  Scenario: --openflow13 option
    When I successfully run `trema run openflow_version.rb --openflow13 -c trema.conf -d`
    And sleep 5
    Then the file "OpenflowVersion.log" should contain "ofp_version = 4"

  @sudo
  Scenario: --no-openflow13 option
    When I successfully run `trema run openflow_version.rb --no-openflow13 -c trema.conf -d`
    And sleep 5
    Then the file "OpenflowVersion.log" should contain "ofp_version = 1"

  @sudo
  Scenario: the default OpenFlow version is 1.0
    When I successfully run `trema run openflow_version.rb -c trema.conf -d`
    And sleep 5
    Then the file "OpenflowVersion.log" should contain "ofp_version = 1"
