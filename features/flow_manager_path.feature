
Feature: flow manager path class function

  Flow_manager is a API for setting a path easily.
  To set a path, you just need to make a path object and add hop objects, which represent switches, to show how to route your data.
  This "path" class has 5 attributes, match, priority, idle_timeout, hard_timeout, and hops, and accesser functions.
  Also it has functions to set a path you defined and teardown a path. 
  
  Background:
  Given a file named "flow_manager.conf" with:
    """
    vswitch{ datapath_id "0x1" }
    vswitch{ datapath_id "0x2" }
      
    vhost("host1") { ip "192.168.0.1" }
    vhost("host2") { ip "192.168.0.2" }
      
    link "0x1","0x2"
    link "host1","0x1"
    link "host2","0x2"
    
    event :port_status => "FlowManagerController", :packet_in => "FlowManagerController", :state_notify => "FlowManagerController"
    """
  
  Given a file named "flow_manager_simple.conf" with:
    """
    vswitch{ datapath_id "0x1" }
    vswitch{ datapath_id "0x2" }
       
    link 0x1, 0x2
    
    event :port_status => "FlowManagerController", :packet_in => "FlowManagerController", :state_notify => "FlowManagerController"
    """
    
  @slow_process
  Scenario: Be able to create a path with options
    Given The default aruba timeout is 30 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_path_test.rb -c tmp/aruba/flow_manager_simple.conf`
    When *** sleep 1 ***
    Then the output should contain: 
    """
    path.priority1:65535
    path.idle_timeout1:30
    path.hard_timeout1:30
    path.match1:wildcards = 0(none), in_port = 1, dl_src = 00:00:00:00:00:01, dl_dst = 00:00:00:00:00:02, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 17, nw_src = 192.168.0.1/32, nw_dst = 192.168.0.2/32, tp_src = 10, tp_dst = 20
    """
    
  @slow_process
  Scenario: Be able to create a path with no options
    Given The default aruba timeout is 30 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_path_test.rb -c tmp/aruba/flow_manager_simple.conf`
    When *** sleep 1 ***
    Then the output should contain: 
    """
    path2.priority1:60000
    path2.idle_timeout1:15
    path2.hard_timeout1:30
    path2.match1:wildcards = 0(none), in_port = 1, dl_src = 00:00:00:00:00:01, dl_dst = 00:00:00:00:00:02, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 17, nw_src = 192.168.0.1/32, nw_dst = 192.168.0.2/32, tp_src = 10, tp_dst = 20
    """
  
  @slow_process
  Scenario: Be able to add a hop to a path by "<<"
    Given The default aruba timeout is 30 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_path_test.rb -c tmp/aruba/flow_manager_simple.conf`
    When *** sleep 1 ***
    Then the output should contain: 
    """
    arrHops[0].datapath_id1:2
    arrHops[0].in_port1:2
    arrHops[0].out_port1:3
    arrHops[0].actions1:[#<Trema::SendOutPort:
    """
    Then the output should contain: 
    """
    @port_number=1, @max_len=65535
    """
    
  @slow_process
  Scenario: Be able to add a hop to a path by "add" function
    Given The default aruba timeout is 30 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_path_test.rb -c tmp/aruba/flow_manager_simple.conf`
    When *** sleep 1 ***
    Then the output should contain: 
    """
    arrHops[1].datapath_id1:1
    arrHops[1].in_port1:1
    arrHops[1].out_port1:2
    arrHops[1].actions1:false

    """      
  @slow_process
  Scenario: Be able to setup a path by "setup" function
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_path_setup_test.rb -c tmp/aruba/flow_manager.conf -d`
    When *** sleep 3 ***
    When I send 1 packet from host1 to host2
    When I run `trema show_stats host1 --tx`
    Then the output from "trema show_stats host1 --tx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
    When I run `trema show_stats host2 --rx`
    Then the output from "trema show_stats host2 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
    When *** sleep 15 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:65535
      path.idle:15
      path.hard_timeout:30
      datapath_id:1
      :in_port:2
      :out_port:1
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:65535
      path.idle:15
      path.hard_timeout:30
      datapath_id:2
      :in_port:2
      :out_port:1
      """
    Then the file "tmp/log/FlowManagerController.log" should contain "path.match:wildcards = 0x3820ff(all), in_port = 0, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SendOutPort"
    Then the file "tmp/log/FlowManagerController.log" should contain "@port_number=1, @max_len=256"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetEthSrcAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain: 
    """
    @string="11:22:33:44:55:66"
    """
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetEthDstAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain: 
    """
    @string="22:33:44:55:66:77"
    """
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetIpSrcAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain "IPv4:192.168.1.1/255.255.255.255"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetIpDstAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain "IPv4:192.168.2.1/255.255.255.255"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetIpTos"
    Then the file "tmp/log/FlowManagerController.log" should contain "@type_of_service=32"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetTransportSrcPort"
    Then the file "tmp/log/FlowManagerController.log" should contain "@port_number=5555"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetTransportDstPort"
    Then the file "tmp/log/FlowManagerController.log" should contain "@port_number=6666"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetVlanVid"
    Then the file "tmp/log/FlowManagerController.log" should contain "@vlan_id=1"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetVlanPriority"
    Then the file "tmp/log/FlowManagerController.log" should contain "@vlan_priority=7"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::StripVlanHeader"
    Then the file "tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"
  
  @slow_process
  Scenario: Be able to setup multiple pathes by "setup" function
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_multiple_path_setup_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:10000
      path.idle:13
      path.hard_timeout:31
      datapath_id:1
      :in_port:1
      :out_port:2
      :actions:false
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:10000
      path.idle:13
      path.hard_timeout:31
      datapath_id:2
      :in_port:3
      :out_port:4
      :actions:false
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:20000
      path.idle:12
      path.hard_timeout:32
      datapath_id:1
      :in_port:5
      :out_port:6
      :actions:false
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:20000
      path.idle:12
      path.hard_timeout:32
      datapath_id:2
      :in_port:7
      :out_port:8
      :actions:false
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:30000
      path.idle:11
      path.hard_timeout:33
      datapath_id:1
      :in_port:9
      :out_port:10
      :actions:false
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:30000
      path.idle:11
      path.hard_timeout:33
      datapath_id:2
      :in_port:11
      :out_port:12
      :actions:false
      """
      
  @slow_process
  Scenario: Be able to get path duplication status. It occures when you set the same path.
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_path_setup_duplicate_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = conflicted entry)***"
  
    
  @slow_process
  Scenario: Be able to teardown a path by "teradown" function
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_path_teardown_by_manual_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 10 ***
    Then the file "tmp/log/FlowManagerController.log" should contain " ***Path teardown completed ( reason = manually requested)***"
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:65535
      path.idle:30
      path.hard_timeout:60
      datapath_id:1
      :in_port:1
      :out_port:2
      """
        
  @slow_process
  Scenario: Be able to teardown multiple pathes by "teradown" function
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_multiple_path_teardown_by_manual_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 10 ***
    Then the file "tmp/log/FlowManagerController.log" should contain " ***Path teardown completed ( reason = manually requested)***"
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:65535
      path.idle:31
      path.hard_timeout:30
      datapath_id:2
      :in_port:3
      :out_port:4
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.priority:65535
      path.idle:30
      path.hard_timeout:30
      datapath_id:1
      :in_port:1
      :out_port:2
      """

