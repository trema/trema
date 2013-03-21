Feature: topology manager command line usage
  
  Topology manager is automatically started, when a client use the topology API.
  But it is also possible to manually start topology manager.
  Some behavior of topology manager can be controlled
  by manually starting topology manager with additional options. 
  
  Example: Default enable discovery.
  
      $ trema run "./objects/topology/topology --always_run_discovery" -d
  
  Topology manager's link discovery feature is disable by default, 
  and client need to enable the feature by topology API calls. 
  By manually starting topology manager with `--always_run_discovery` option, 
  topology manager will immediately start discovering links. 
  
  This option can also be useful, when migrating legacy apps/topology application, 
  where link discovery was not enabled by API, 
  but was enabled by running a separate process. 
  Specifying this option will be equivalent to starting discovery daemon process 
  in leagacy apps/topology.

  Scenario Outline: Show topology usage
    When I successfully run `../../objects/topology/topology <argument>`
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
      | argument |
      | --help   |
      | -h       |
