Feature: dumper help

  As a Trema user
  I want to see the help message of dumper command
  So that I can learn how to use dumper

  Scenario: dumper --help
    When I try to run "./objects/examples/dumper/dumper --help"
    Then the output should be:
      """
      OpenFlow Event Dumper.
      Usage: dumper [OPTION]...

        -n, --name=SERVICE_NAME     service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """

  Scenario: dumper -h
    When I try to run "./objects/examples/dumper/dumper -h"
    Then the output should be:
      """
      OpenFlow Event Dumper.
      Usage: dumper [OPTION]...

        -n, --name=SERVICE_NAME     service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """
