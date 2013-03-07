
Feature: flow manager hop class functions

  Flow_manager is a API for setting a path easily.
  To set a path, you just need to make a path object and add hop objects, which represent switches, to show how to route your data.
  This "hop" class has 4 attributes, datapath_id, in_port, out_port, and actions and accesser functions.
  
  Background:
  Given a file named "flow_manager_simple.conf" with:
    """
    vswitch{ datapath_id "0x1" }
    vswitch{ datapath_id "0x2" }
       
    link 0x1, 0x2
    
    event :port_status => "FlowManagerController", :packet_in => "FlowManagerController", :state_notify => "FlowManagerController"
    """
    
  @slow_process
  Scenario: Be able to create hops with no actions
    Given The default aruba timeout is 30 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_hop_test.rb -c tmp/aruba/flow_manager_simple.conf`
    When *** sleep 3 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "hop1.datapath_id:1"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop1.in_port:1"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop1.out_port:2"    
    Then the file "tmp/log/FlowManagerController.log" should contain "hop2.datapath_id:2"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop2.in_port:2"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop2.out_port:3"
    
  @slow_process
  Scenario: Be able to create hops with actions
    Given The default aruba timeout is 30 seconds
    Given I cd to "../.."
    When I run `trema run src/flow_manager/flow_manager_test/flow_manager_hop_test.rb -c tmp/aruba/flow_manager_simple.conf`
    When *** sleep 3 ***
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SendOutPort"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetEthSrcAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetEthDstAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetIpSrcAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetIpDstAddr"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetIpTos"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetTransportSrcPort"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetTransportDstPort"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetVlanVid"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::SetVlanPriority"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::StripVlanHeader"
    Then the file "tmp/log/FlowManagerController.log" should contain "Trema::VendorAction"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[0].max_len:256"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[0].port_number:1"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[3].ip_address:#<IPAddr: IPv4:192.168.1.1/255.255.255.255>"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[4].ip_address:#<IPAddr: IPv4:192.168.1.1/255.255.255.255>"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[5].type_of_service:32"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[6].port_number:5555"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[7].port_number:5555"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[8].vlan_id:1"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[9].vlan_priority:7"
    Then the file "tmp/log/FlowManagerController.log" should contain "hop3.actions[11].vendor_id:19711"
    
    