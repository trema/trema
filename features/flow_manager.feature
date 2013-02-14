Feature: flow_manager tests

  As a Trema user
  I want to use flow_manager to make a flow to OpenFlow switches
  So that I can debug my Trema apps
  
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
    """
    
  Given a file named "cflow_manager.conf" with:
    """
    vswitch("switch1") { datapath_id "0x1" }
    vswitch("switch2") { datapath_id "0x2" }
    
    link "switch1", "switch2"
    
    event :port_status => "flow_manager_test", :packet_in => "flow_manager_test", :state_notify => "flow_manager_test"
    """
  @slow_process
  Scenario: Run "flow_manager c test" C example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run -c cflow_manager.conf ../../objects/examples/flow_manager_example/flow_manager_example --flowmanager`
    When *** sleep 10 ***
    Then the file "../../tmp/log/flow_manager_example.log" should contain " *** Path setup completed ( status = succeeded, user_data = (nil) )"
 
 
  @slow_process
  Scenario: Run "flow_manager_multiple_path_setup_test" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_multiple_path_setup_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 30 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.priority:60000"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.priority:65535"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "nw_src = 192.168.0.1/32"  

  @slow_process  
  Scenario: Run "flow_manager path setup test" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_path_setup_test.rb -c flow_manager.conf -d --flowmanager`
    When *** sleep 5 ***
    When I send 1 packet from host1 to host2
    When I run `trema show_stats host1 --tx`
    Then the output from "trema show_stats host1 --tx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
    When I run `trema show_stats host2 --rx`
    Then the output from "trema show_stats host2 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
    When *** sleep 10 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.priority:65535"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.idle:15"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.hard_timeout:30"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.match:wildcards = 0x3820ff(all), in_port = 0, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[0].datapath_id:1"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[0].in_port:2"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[0].out_port:1"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].datapath_id:2"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].in_port:2"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].out_port:1"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].actions:false"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"
 
  @slow_process
  Scenario: Run "flow_manager path setup duplicate" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_path_setup_duplicate_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 20 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = conflicted entry)***"
  
  @slow_process
  Scenario: Run "flow_manager path teardown by manual" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_path_teardown_by_manual_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 20 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain " ***Path teardown completed ( reason = manually requested)***"
  
  @slow_process
  Scenario: Run "flow_manager module setup" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_module_setup_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 20 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.priority:65535"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.idle:15"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.hard_timeout:30"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.match:wildcards = 0x3820fe(dl_src|dl_dst|dl_type|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(32)|nw_dst(32)|tp_src|tp_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[0].datapath_id:1"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[0].in_port:1"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[0].out_port:2"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrAction1[1].max_len():65535"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].datapath_id:2"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].in_port:2"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].out_port:1"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "arrHops[1].actions:false"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"
  
  @slow_process
  Scenario: Run "flow_manager module lookup" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_module_lookup_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 20 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "***lookup start"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.priority:65535"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.idle:10"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.hard_timeout:30"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "path.match:wildcards = 0x3820fe(dl_src|dl_dst|dl_type|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(32)|nw_dst(32)|tp_src|tp_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"
  
  @slow_process
  Scenario: Run "flow_manager module teardown" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_module_teardown_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 20 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "teardown bool:true"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = manually requested)***"    
  
  @slow_process
  Scenario: Run "flow_manager module teardown by match" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_module_teardown_by_match_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 20 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "teardown bool:true"
    Then the file "../../tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = manually requested)***"    
    
  @slow_process
  Scenario: Run "flow_manager_init_finalize_test" Ruby example
    Given The default aruba timeout is 60 seconds
    Given I run `trema run ../../src/flow_manager/flow_manager_test/flow_manager_init_finalize_test.rb -c flow_manager_simple.conf -d --flowmanager`
    When *** sleep 20 ***
    Then the file "../../tmp/log/FlowManagerController.log" should contain "false"