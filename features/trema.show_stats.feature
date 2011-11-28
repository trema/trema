Feature: show network stats with `trema show_stats' command

  As a Trema user
  I want to get packet stats with `trema show_stats' command
  So that I can easily debug trema applications


  Background:
    Given I try trema run "learning_switch" example with following configuration (backgrounded):
      """
      vswitch { datapath_id "0xabc" }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "0xabc", "host1"
      link "0xabc", "host2"
      """


  Scenario: show_stats
    When I send 1 packet from host1 to host2
    Then the stats of "host1" should be:
      """
      Sent packets:
      ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
      192.168.0.2,1,192.168.0.1,1,1,50
      Received packets:
      """


  Scenario: show_stats --tx
    When I send 1 packet from host1 to host2
    Then the tx stats of "host1" should be:
      """
      ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
      192.168.0.2,1,192.168.0.1,1,1,50
      """


  Scenario: show_stats --rx
    When I send 1 packet from host1 to host2
    Then the rx stats of "host2" should be:
      """
      ip_dst,tp_dst,ip_src,tp_src,n_pkts,n_octets
      192.168.0.2,1,192.168.0.1,1,1,50
      """


  Scenario: trema help show_stats
    When I try to run "./trema help show_stats"
    Then the output should be:
      """
      Usage: ./trema show_stats [OPTIONS ...]
          -t, --tx
          -r, --rx

          -h, --help
          -v, --verbose
      """
