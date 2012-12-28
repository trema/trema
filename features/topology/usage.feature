Feature: topology manager usage
  
  Topology manager is usually automatically started, 
  when a client tries to use the topology API, and no command line options 
  is required.  
  But, it is also possible to manually start topology specifying some 
  command line options to topology manager, in certain cases.  
  
  Example: Default enable discovery.
  
      $ trema run "./objects/topology/topology --always_run_discovery" -d
  
  Topology manager's link discovery feature is disable by default, 
  and client need to enable the feature through topology API calls. 
  By manually starting topology manager with `--always_run_discovery` option, 
  topology manager can start discovering links without user applications request. 
  
  Using this option, topology manager will start discovering link topology 
  as soon as possible, so it may speed up the time for the link topology 
  information to stabilize.  
  This option can also be useful, when migrating legacy apps/topology application, 
  where link discovery was not controlled by API, 
  but was contolled by running a separate process. 
  Specifying this option will be equivalent to starting discovery daemon process 
  in leagacy apps/topology.  
  
  Please see the usage message in the following scenarios 
  for available command line options.

  Scenario Outline: Show topology usage
    When I run `../../objects/topology/topology <arg>`
    Then the output should contain:
      """
      topology manager
      Usage: topology [OPTION]...
      
        -w, --liveness_wait=SEC         subscriber liveness check interval
        -e, --liveness_limit=COUNT      set liveness check error threshold
        -a, --always_run_discovery      discovery will always be enabled
        -m, --lldp_mac_dst=MAC_ADDR     destination Mac address for sending LLDP
        -i, --lldp_over_ip              send LLDP messages over IP
        -o, --lldp_ip_src=IP_ADDR       source IP address for sending LLDP over IP
        -r, --lldp_ip_dst=IP_ADDR       destination IP address for sending LLDP over IP
        -n, --name=SERVICE_NAME         service name
        -d, --daemonize                 run in the background
        -l, --logging_level=LEVEL       set logging level
        -g, --syslog                    output log messages to syslog
        -f, --logging_facility=FACILITY set syslog facility
        -h, --help                      display this help and exit
      """

    Examples: 
      | arg    |
      | --help |
      | -h     |
