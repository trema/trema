Feature: show_stats command

  In order to get the stats of sent/received packets
  As a developer using Trema
  I want to execute "trema show_stats" command

  Background:
    Given a file named "learning_switch.conf" with:
      """
      vswitch { datapath_id 0xabc }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "0xabc", "host1"
      link "0xabc", "host2"
      """
    And I run `trema run ../../src/examples/learning_switch/learning-switch.rb -c learning_switch.conf -d`

  @slow_process
  Scenario: show_stats hostname
    When I run `trema send_packets --source host1 --dest host2`
     And I run `trema show_stats host1`
    Then the output should contain:
      """
      Sent packets:
      ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
      192.168.0.2,1,192.168.0.1,1,1,50
      Received packets:
      """

  @slow_process
  Scenario: show_stats hostname --tx
    When I run `trema send_packets --source host1 --dest host2`
     And I run `trema show_stats host1 --tx`
    Then the output should contain:
      """
      ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
      192.168.0.2,1,192.168.0.1,1,1,50
      """

  @slow_process
  Scenario: show_stats hostname --rx
    When I run `trema send_packets --source host1 --dest host2`
     And *** sleep 1 ***
     And I run `trema show_stats host2 --rx`
    Then the output should contain:
      """
      ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
      192.168.0.2,1,192.168.0.1,1,1,50
      """

  Scenario: argument error
    Then I run `trema show_stats`
    Then the output should contain:
      """
      host is required
      """

  Scenario: unknown host error
    Then I run `trema show_stats NO_SUCH_HOST`
    Then the output should contain:
      """
      unknown host: NO_SUCH_HOST
      """
