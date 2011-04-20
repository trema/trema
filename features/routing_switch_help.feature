Feature: routing_switch help

  As a Trema user
  I want to see the help message of routing_switch command
  So that I can learn how to use routing_switch

  Scenario: routing_switch --help
    When I try to run "./objects/examples/routing_switch/routing_switch --help"
    Then the output should be:
      """
      Switching HUB.
      Usage: routing_switch [OPTION]...

        -i, --idle_timeout=TIMEOUT  Idle timeout value of flow entry
        -n, --name=SERVICE_NAME     service name
        -t, --topology=SERVICE_NAME topology service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """

  Scenario: routing_switch -h
    When I try to run "./objects/examples/routing_switch/routing_switch -h"
    Then the output should be:
      """
      Switching HUB.
      Usage: routing_switch [OPTION]...

        -i, --idle_timeout=TIMEOUT  Idle timeout value of flow entry
        -n, --name=SERVICE_NAME     service name
        -t, --topology=SERVICE_NAME topology service name
        -d, --daemonize             run in the background
        -l, --logging_level=LEVEL   set logging level
        -h, --help                  display this help and exit
      """
