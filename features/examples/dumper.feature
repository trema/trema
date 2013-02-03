Feature: "Dumper" sample application

  In order to learn how to dump miscellaneous OpenFlow messages
  As a developer using Trema
  I want to execute "Dumper" sample application

  Background:
    Given a file named "dumper.conf" with:
      """
      vswitch("dumper") { datapath_id 0xabc }

      vhost("host1")
      vhost("host2")

      link "dumper", "host1"
      link "dumper", "host2"
      """

  @slow_process
  Scenario: Run "Dumper" C example
    Given I run `trema run ../../objects/examples/dumper/dumper -c dumper.conf -d`
     And wait until "dumper" is up
    When I send 1 packet from host1 to host2
    Then the file "../../tmp/log/dumper.log" should contain "[switch_ready]"
     And the file "../../tmp/log/dumper.log" should contain "[packet_in]"
     And the file "../../tmp/log/dumper.log" should contain "datapath_id: 0xabc"

  @slow_process
  Scenario: Run "Dumper" Ruby example
    Given I run `trema run ../../src/examples/dumper/dumper.rb -c dumper.conf -d`
     And wait until "Dumper" is up
    When I send 1 packet from host1 to host2
    Then the file "../../tmp/log/Dumper.log" should contain "[switch_ready]"
     And the file "../../tmp/log/Dumper.log" should contain "[packet_in]"
     And the file "../../tmp/log/Dumper.log" should contain "datapath_id: 0xabc"

  Scenario: "dumper --help"
    When I run `../../objects/examples/dumper/dumper --help`
    Then the output should contain exactly:
      """
      OpenFlow Event Dumper.
      Usage: dumper [OPTION]...

        -n, --name=SERVICE_NAME         service name
        -d, --daemonize                 run in the background
        -l, --logging_level=LEVEL       set logging level
        -g, --syslog                    output log messages to syslog
        -f, --logging_facility=FACILITY set syslog facility
        -h, --help                      display this help and exit

      """

  Scenario: "dumper -h"
    When I run `../../objects/examples/dumper/dumper -h`
    Then the output should contain exactly:
      """
      OpenFlow Event Dumper.
      Usage: dumper [OPTION]...

        -n, --name=SERVICE_NAME         service name
        -d, --daemonize                 run in the background
        -l, --logging_level=LEVEL       set logging level
        -g, --syslog                    output log messages to syslog
        -f, --logging_facility=FACILITY set syslog facility
        -h, --help                      display this help and exit

      """
