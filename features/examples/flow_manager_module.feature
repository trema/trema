
Feature: flow manager module functions

  Flow_manager is a API for setting a path easily.
  To set a path, you just need to make a path object and add hop objects, which represent switches, to show how to route your data.
  This "module" has functions to set a path you defined and teardown a path.
  And also it has support function, lookup path. 
  
  Background:
  Given a file named "flow_manager_simple.conf" with:
    """
    vswitch{ datapath_id "0x1" }
    vswitch{ datapath_id "0x2" }
       
    link 0x1, 0x2
    
    event :port_status => "FlowManagerController", :packet_in => "FlowManagerController", :state_notify => "FlowManagerController"
    """
    
  Given a file named "flow_manager_simple2.conf" with:
    """
    event :port_status => "FlowManagerController2", :packet_in => "FlowManagerController2", :state_notify => "FlowManagerController2"
    """
    
  Given a file named "cflow_manager.conf" with:
    """
    vswitch("switch1") { datapath_id "0x1" }
    vswitch("switch2") { datapath_id "0x2" }
    
    link "switch1", "switch2"
    
    event :port_status => "flow_manager_test", :packet_in => "flow_manager_test", :state_notify => "flow_manager_test"
    """
  @slow_process
  Scenario: Be able to setup a path by "setup" function (Path class's setup function uses this function to setup a path.)
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_module_setup_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.match:wildcards = 0x3820fe(dl_src|dl_dst|dl_type|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(32)|nw_dst(32)|tp_src|tp_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0
      path.priority:65535
      path.idle:15
      path.hard_timeout:30
      datapath_id:1
      :in_port:1
      :out_port:2
      :actions:[#<Trema::StripVlanHeader
      """
    Then the file "tmp/log/FlowManagerController.log" should contain:
      """
      path.match:wildcards = 0x3820fe(dl_src|dl_dst|dl_type|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(32)|nw_dst(32)|tp_src|tp_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0
      path.priority:65535
      path.idle:15
      path.hard_timeout:30
      datapath_id:2
      :in_port:2
      :out_port:1
      :actions:false
      """
    Then the file "tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"
    
  @slow_process
  Scenario: Be able to setup a path by "setup" function with 2 controller with one flow_manager deamon 
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_module_setup_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 3 ***
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_module_setup_test2.rb -c tmp/aruba/flow_manager_simple2.conf -d`
    When *** sleep 20 *** 
    Then the file "tmp/log/FlowManagerController2.log" should contain "[info] flow_manager is already existed."  
    Then the file "tmp/log/FlowManagerController2.log" should contain "[info] **** Path setup completed ( status = conflicted entry)***"  
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_module_setup_test2.rb -c tmp/aruba/flow_manager_simple2.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController2.log" should contain "[info] flow_manager is already existed."  
    Then the file "tmp/log/FlowManagerController2.log" should contain "[info] **** Path setup completed ( status = succeeded)***"  
  
  @slow_process
  Scenario: Be able to teardown a path by "teardown" function. 
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_module_teardown_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "tmp/log/FlowManagerController.log" should contain "teardown bool:true"
    Then the file "tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = manually requested)***"    
  
  @slow_process
  Scenario: Be able to teardown a path by "teardown_by_match" function.
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_module_teardown_by_match_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "tmp/log/FlowManagerController.log" should contain "teardown bool:true"
    Then the file "tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = manually requested)***"  
  
  @slow_process
  Scenario: Be able to lookup a path by "lookup path" function.
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_module_lookup_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "**** Path setup completed ( status = succeeded)***"
    Then the file "tmp/log/FlowManagerController.log" should contain "***lookup start"
    Then the file "tmp/log/FlowManagerController.log" should contain "path.priority:65535"
    Then the file "tmp/log/FlowManagerController.log" should contain "path.idle:10"
    Then the file "tmp/log/FlowManagerController.log" should contain "path.hard_timeout:30"
    Then the file "tmp/log/FlowManagerController.log" should contain "path.match:wildcards = 0x3820fe(dl_src|dl_dst|dl_type|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(32)|nw_dst(32)|tp_src|tp_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0"
    Then the file "tmp/log/FlowManagerController.log" should contain "***Path teardown completed ( reason = timeout)***"  
  
  @slow_process
  Scenario: Be able to initialize and finalize flow_manager library.
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_init_finalize_test.rb -c tmp/aruba/flow_manager_simple.conf -d`
    When *** sleep 20 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "false"
    
  @slow_process
  Scenario: Be able to run this function even in c language.
    Given The default aruba timeout is 60 seconds
    Given I cd to "../.."
    When I run `trema run -c tmp/aruba/cflow_manager.conf objects/examples/flow_manager_example/flow_manager_example -d`
    When *** sleep 15 ***
    Then the file "tmp/log/flow_manager_example.log" should contain " *** Path setup completed ( status = succeeded, user_data = (nil) )"
    