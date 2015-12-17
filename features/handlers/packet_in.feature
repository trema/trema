Feature: packet_in handler
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      vhost('host1') { ip '192.168.0.1' }
      vhost('host2') { ip '192.168.0.2' }
      link '0xabc', 'host1'
      link '0xabc', 'host2'
      """

  @sudo
  Scenario: invoke packet_in handler
    Given a file named "packet_in_controller.rb" with:
      """ruby
      class PacketInController < Trema::Controller
        def packet_in(dpid, message)
          logger.info "new packet_in (dpid = #{dpid.to_hex})"
        end
      end
      """

    And I successfully run `trema run packet_in_controller.rb -c trema.conf -d`
    And sleep 3
    When I successfully run `trema send_packets --source host1 --dest host2`
    Then the file "PacketInController.log" should contain "new packet_in (dpid = 0xabc)"

  @sudo
  Scenario: invoke packet_in handler (OpenFlow 1.3)
    Given a file named "packet_in_controller.rb" with:
      """ruby
      class PacketInController < Trema::Controller
        def switch_ready(dpid)
          send_flow_mod_add(
            dpid,
            match: Match.new,
            instructions: Apply.new(SendOutPort.new(:controller))
          )
        end

        def packet_in(dpid, message)
          logger.info "new packet_in (dpid = #{dpid.to_hex})"
        end
      end
      """
    And I successfully run `trema run packet_in_controller.rb -c trema.conf --openflow13 -d`
    And sleep 3
    When I successfully run `trema send_packets --source host1 --dest host2`
    Then the file "PacketInController.log" should contain "new packet_in (dpid = 0xabc)"
