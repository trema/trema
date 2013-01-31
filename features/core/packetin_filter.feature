Feature: packetin_filter help

  As a Trema user
  I want to see the help message of packetin_filter command
  So that I can learn how to use packetin_filter

  @wip
  Scenario: packetin_filter --help
    When I try to run "./objects/packetin_filter/packetin_filter --help"
    Then the output should be:
      """
      OpenFlow Packet in Filter.
      Usage: packetin_filter [OPTION]... [PACKETIN-FILTER-RULE]...

        -n, --name=SERVICE_NAME         service name
        -d, --daemonize                 run in the background
        -l, --logging_level=LEVEL       set logging level
        -g, --syslog                    output log messages to syslog
        -f, --logging_facility=FACILITY set syslog facility
        -h, --help                      display this help and exit

      PACKETIN-FILTER-RULE:
        match-type::destination-service-name

      match-type:
        lldp                            LLDP ethernet frame type and priority is 0x8000
        packet_in                       any packet and priority is zero

      destination-service-name          destination service name
      """

  @wip
  Scenario: packetin_filter -h
    When I try to run "./objects/packetin_filter/packetin_filter -h"
    Then the output should be:
      """
      OpenFlow Packet in Filter.
      Usage: packetin_filter [OPTION]... [PACKETIN-FILTER-RULE]...

        -n, --name=SERVICE_NAME         service name
        -d, --daemonize                 run in the background
        -l, --logging_level=LEVEL       set logging level
        -g, --syslog                    output log messages to syslog
        -f, --logging_facility=FACILITY set syslog facility
        -h, --help                      display this help and exit

      PACKETIN-FILTER-RULE:
        match-type::destination-service-name

      match-type:
        lldp                            LLDP ethernet frame type and priority is 0x8000
        packet_in                       any packet and priority is zero

      destination-service-name          destination service name
      """
