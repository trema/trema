Feature: send_packets command

  In order to send/receive packets between virtual hosts
  As a developer using Trema
  I want to execute "trema send_packets" command

  Background:
    Given a file named "learning_switch.conf" with:
      """
      vswitch("learning") { datapath_id 0xabc }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "learning", "host1"
      link "learning", "host2"
      """
    And I run `trema run ../../src/examples/learning_switch/learning-switch.rb -c learning_switch.conf -d`

  @slow_process
  Scenario: Sending packets
    When I run `trema send_packets --source host1 --dest host2 --n_pkts=2`
     And I run `trema send_packets --source host2 --dest host1 --n_pkts=1`
    Then the total number of tx packets should be:
      | host1 | host2 |
      |     2 |     1 |
    And the total number of rx packets should be:
      | host1 | host2 |
      |     1 |     2 |

  @slow_process
  Scenario: unknown host error (--source)
    When I run `trema send_packets --source NO_SUCH_HOST --dest host2`
    Then the output should contain:
      """
      unknown host: NO_SUCH_HOST
      """

  @slow_process
  Scenario: unknown host error (--dest)
    When I run `trema send_packets --source host1 --dest NO_SUCH_HOST`
    Then the output should contain:
      """
      unknown host: NO_SUCH_HOST
      """
