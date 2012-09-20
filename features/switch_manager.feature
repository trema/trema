Feature: switch_manager help

  As a Trema user
  I want to see the help message of switch_manager command
  So that I can learn how to use switch_manager

  @wip
  Scenario: switch_manager --help
    When I try to run "./objects/switch_manager/switch_manager --help"
    Then the output should be:
      """
      OpenFlow Switch Manager.
      Usage: switch_manager [OPTION]... [-- SWITCH_MANAGER_OPTION]...

        -s, --switch=PATH               the command path of switch
        -n, --name=SERVICE_NAME         service name
        -p, --port=PORT                 server listen port (default 6633)
        -d, --daemonize                 run in the background
        -l, --logging_level=LEVEL       set logging level
        -g, --syslog                    output log messages to syslog
        -f, --logging_facility=FACILITY set syslog facility
        -h, --help                      display this help and exit
      """

  @wip
  Scenario: switch_manager -h
    When I try to run "./objects/switch_manager/switch_manager -h"
    Then the output should be:
      """
      OpenFlow Switch Manager.
      Usage: switch_manager [OPTION]... [-- SWITCH_MANAGER_OPTION]...

        -s, --switch=PATH               the command path of switch
        -n, --name=SERVICE_NAME         service name
        -p, --port=PORT                 server listen port (default 6633)
        -d, --daemonize                 run in the background
        -l, --logging_level=LEVEL       set logging level
        -g, --syslog                    output log messages to syslog
        -f, --logging_facility=FACILITY set syslog facility
        -h, --help                      display this help and exit
      """
