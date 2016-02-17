Feature: reset_stats
  Background:
    Given I set the environment variables to:
      | variable         | value |
      | TREMA_LOG_DIR    | .     |
      | TREMA_PID_DIR    | .     |
      | TREMA_SOCKET_DIR | .     |
    And a file named "flood.rb" with:
      """ruby
      class Flood < Trema::Controller
        def packet_in(datapath_id, message)
          send_packet_out(
            datapath_id,
            packet_in: message,
            actions: SendOutPort.new(:flood)
          )
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

  @sudo
  Scenario: run controller_file
    When I successfully run `trema run flood.rb -c trema.conf -d` 
    And I successfully run `trema send_packets --source host1 --dest host2`
    And I successfully run `trema reset_stats`
    Then I successfully run `trema show_stats host1`
    And the output from "trema show_stats host1" should contain exactly ""
    And I successfully run `trema show_stats host2`
    And the output from "trema show_stats host2" should contain exactly ""
