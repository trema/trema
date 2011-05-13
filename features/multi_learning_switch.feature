Feature: control multiple openflow switches using multi_learning_switch

  As a Trema user
  I want to control multiple openflow switches using multi_learning_switch
  So that I can send and receive packets

  Background:
    Given I terminated all trema services

  Scenario: One openflow switch, two servers
    When I try trema run with following configuration:
      """
      vswitch("mlsw1") {
        datapath_id "0x1"
      }

      vswitch("mlsw2") {
        datapath_id "0x2"
      }

      vswitch("mlsw3") {
        datapath_id "0x3"
      }

      vswitch("mlsw4") {
        datapath_id "0x4"
      }

      vhost ("host1") {
        ip "192.168.0.1"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:01"
      }

      vhost ("host2") {
        ip "192.168.0.2"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:02"
      }

      vhost ("host3") {
        ip "192.168.0.3"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:03"
      }

      vhost ("host4") {
        ip "192.168.0.4"
        netmask "255.255.0.0"
        mac "00:00:00:01:00:04"
      }

      link "mlsw1", "host1"
      link "mlsw2", "host2"
      link "mlsw3", "host3"
      link "mlsw4", "host4"
      link "mlsw1", "mlsw2"
      link "mlsw2", "mlsw3"
      link "mlsw3", "mlsw4"

      app {
        path "./objects/examples/multi_learning_switch/multi_learning_switch"
      }
      """
      And wait until "multi_learning_switch" is up

    When I try to run "./trema send_packets --source host1 --dest host2 --n_pkts 2"
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     0 |     0 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     0 |     2 |     0 |     0 |

    When I try to run "./trema send_packets --source host3 --dest host4 --n_pkts 3"
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     3 |     0 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     0 |     2 |     0 |     3 |

    And I try to run "./trema send_packets --source host4 --dest host1 --n_pkts 2"
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     0 |     3 |     2 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     0 |     3 |

    And I try to run "./trema send_packets --source host2 --dest host3 --n_pkts 4"
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     4 |     3 |     2 |
    And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     4 |     3 |

    And I try to run "./trema send_packets --source host1 --dest host4 --n_pkts 1"
    Then the total number of tx packets should be:
      | host1 | host2 | host3 | host4 |
      |     3 |     4 |     3 |     2 |
     And the total number of rx packets should be:
      | host1 | host2 | host3 | host4 |
      |     2 |     2 |     4 |     4 |

   And I terminated all trema services
