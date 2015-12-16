Feature: delete_link
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |

  @sudo
  Scenario: trema delete_link 0xabc host1
    Given a file named "packet_in_controller.rb" with:
      """ruby
      class PacketInController < Trema::Controller
        def packet_in(dpid, message)
          logger.info 'new packet_in'
        end
      end
      """
    And a file named "trema.conf" with:
      """ruby
      vswitch { datapath_id 0xabc }
      vhost('host1') { ip '192.168.0.1' }
      vhost('host2') { ip '192.168.0.2' }
      link '0xabc', 'host1'
      link '0xabc', 'host2'
      """
    And I run `trema run packet_in_controller.rb -c trema.conf -d`
    When I successfully run `trema delete_link 0xabc host1`
    And I successfully run `trema send_packets --source host1 --dest host2`
    Then the file "PacketInController.log" should not contain "new packet_in"

  @wip
  Scenario: "link not found" error
