Feature: Dump openflow events with dumper

  As a Trema user
  I want to dump OpenFlow events with dumper example application
  So that I can visualize OpenFlow messages


  Scenario: Dump packet_in events
    Given I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      vswitch("dumper") { datapath_id "0xabc" }

      vhost("host1") {
        ip "192.168.0.1"
        mac "00:00:00:00:00:01"
      }
      vhost("host2") {
        ip "192.168.0.2"
        mac "00:00:00:00:00:02"
      }

      link "dumper", "host1"
      link "dumper", "host2"
      """
      And wait until "dumper" is up
    When I try to run "./trema send_packets --source host1 --dest host2 --length=0"
      And I terminated all trema services
    Then the output should include:
      """
      [packet_in]
        datapath_id: 0xabc
      """


  Scenario: Dump packet_in events (Ruby)
    Given I try trema run "./src/examples/dumper/dumper.rb" with following configuration (backgrounded):
      """
      vswitch("dumper") { datapath_id "0xabc" }

      vhost("host1") {
        ip "192.168.0.1"
        mac "00:00:00:00:00:01"
      }
      vhost("host2") {
        ip "192.168.0.2"
        mac "00:00:00:00:00:02"
      }

      link "dumper", "host1"
      link "dumper", "host2"
      """
      And wait until "Dumper" is up
    When I try to run "./trema send_packets --source host1 --dest host2 --length=0"
      And I terminated all trema services
    Then the output should include:
      """
      [packet_in]
        datapath_id: 0xabc
      """


  Scenario: dumper --help
    When I try to run "./objects/examples/dumper/dumper --help" (log = "dumper_help.log")
    Then the log file "dumper_help.log" should be:
      """
      OpenFlow Event Dumper.
      Usage: dumper [OPTION]...

        -n, --name=SERVICE_NAME     service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """


  Scenario: dumper -h
    When I try to run "./objects/examples/dumper/dumper -h" (log = "dumper_h.log")
    Then the log file "dumper_h.log" should be:
      """
      OpenFlow Event Dumper.
      Usage: dumper [OPTION]...

        -n, --name=SERVICE_NAME     service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """
