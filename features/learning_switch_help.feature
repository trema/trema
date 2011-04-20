Feature: learning_switch help

  As a Trema user
  I want to see the help message of learning_switch command
  So that I can learn how to use learning_switch

  Scenario: learning_switch --help
    When I try to run "./objects/examples/learning_switch/learning_switch --help"
    Then the output should be:
      """
      The Implementation of Learning Switch.
      Usage: learning_switch [OPTION]...

        -n, --name=SERVICE_NAME     service name
        -i, --datapath_id=ID        datapath ID
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """

  Scenario: learning_switch -h
    When I try to run "./objects/examples/learning_switch/learning_switch -h"
    Then the output should be:
      """
      The Implementation of Learning Switch.
      Usage: learning_switch [OPTION]...

        -n, --name=SERVICE_NAME     service name
        -i, --datapath_id=ID        datapath ID
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """
