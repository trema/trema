Feature: Dumper help

  As a Trema user
  I want to see the help message of dumper command
  So that I can learn how to use dumper

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
