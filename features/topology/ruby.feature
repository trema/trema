Feature: topology Ruby API
  
  Following scenarios show a simple usage example of Topology Ruby API.
  
  Typical usage pattern:
  
  1. Call get_all_{switch, port, link}_status to obtain initial state.
  2. Define switch_status_{up, down} and {port, link}_status_updated handler to
     keep the topology information updated in your application.

  Scenario: Controller receives switch, port, link update notifications when a handler is defined
    Given a file named "TopologyEventTestController.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      vswitch("topology2") { datapath_id "0x2" }
      
      link "topology1", "topology2"
      """
    And a file named "TopologyEventTestController.rb" with:
      """ruby
      class TopologyEventTestController < Controller
        include Topology
        
        def switch_status_up dpid
          info Switch.new( dpid ).to_s
        end
        
        def switch_status_down dpid
          info "Switch %#x is down" % dpid
        end
        
        def port_status_updated port_stat
          info Port.new( port_stat ).to_s
        end
        
        def link_status_updated link_stat
          info Link.new( link_stat ).to_s
        end
      end
      """
    When I successfully run `trema run TopologyEventTestController.rb -d`
    And wait until "topology" is up
    And I successfully run `trema run -c TopologyEventTestController.conf -d`
    And *** sleep 6 ***
    And I successfully run `trema kill topology2`
    And *** sleep 2 ***
    Then the file "../../tmp/log/TopologyEventTestController.log" should contain:
      """
      Switch: 0x1 - {up:true}
      """
    And the file "../../tmp/log/TopologyEventTestController.log" should contain:
      """
      Switch: 0x2 - {up:true}
      """
    And the file "../../tmp/log/TopologyEventTestController.log" should match /Port: 0x1:1 - \{external:false, mac:"[0-9a-f:]+", name:".*", up:true\}/
    And the file "../../tmp/log/TopologyEventTestController.log" should match /Port: 0x2:1 - \{external:false, mac:"[0-9a-f:]+", name:".*", up:true\}/
    And the file "../../tmp/log/TopologyEventTestController.log" should contain:
      """
      Link: (0x1:1)->(0x2:1) - {unstable:false, up:true}
      """
    And the file "../../tmp/log/TopologyEventTestController.log" should contain:
      """
      Link: (0x2:1)->(0x1:1) - {unstable:false, up:true}
      """
    And the file "../../tmp/log/TopologyEventTestController.log" should contain:
      """
      Switch 0x2 is down
      """

  Scenario: Current status can be obtained by get_all_{switch, port, link}_status call with block callback
    Given a file named "GetAllTestControllerB.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      vswitch("topology2") { datapath_id "0x2" }
      
      link "topology1", "topology2"
      """
    And a file named "GetAllTestControllerB.rb" with:
      """ruby
      class GetAllTestControllerB < Controller
        include Topology
        
        oneshot_timer_event :test_start, 0
        
        def test_start
          # Need to manually enable discovery
          # since link_status_updated is not defined.
          send_enable_topology_discovery
        end
        
        oneshot_timer_event :show_topology, 4
        
        def show_topology
          get_all_switch_status do |switches|
            switches.each { |each| info Switch.new( each ).to_s }
          end
          get_all_port_status do |ports|
            ports.each { |each| info Port.new( each ).to_s }
          end
          get_all_link_status do |links|
            links.each { |each| info Link.new( each ).to_s }
          end
        end
      end
      """
    When I successfully run `trema run GetAllTestControllerB.rb -c GetAllTestControllerB.conf -d`
    And wait until "topology" is up
    And *** sleep 6 ***
    Then the file "../../tmp/log/GetAllTestControllerB.log" should contain:
      """
      Switch: 0x1 - {up:true}
      """
    And the file "../../tmp/log/GetAllTestControllerB.log" should contain:
      """
      Switch: 0x2 - {up:true}
      """
    And the file "../../tmp/log/GetAllTestControllerB.log" should match /Port: 0x1:1 - \{external:false, mac:"[0-9a-f:]+", name:".*", up:true\}/
    And the file "../../tmp/log/GetAllTestControllerB.log" should match /Port: 0x2:1 - \{external:false, mac:"[0-9a-f:]+", name:".*", up:true\}/
    And the file "../../tmp/log/GetAllTestControllerB.log" should contain:
      """
      Link: (0x1:1)->(0x2:1) - {unstable:false, up:true}
      """
    And the file "../../tmp/log/GetAllTestControllerB.log" should contain:
      """
      Link: (0x2:1)->(0x1:1) - {unstable:false, up:true}
      """

  Scenario: Current status can be obtained by get_all_{switch, port, link}_status call and handler method callback
    Given a file named "GetAllTestController.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      vswitch("topology2") { datapath_id "0x2" }
      
      link "topology1", "topology2"
      """
    And a file named "GetAllTestController.rb" with:
      """ruby
      class GetAllTestController < Controller
        include Topology
      
        oneshot_timer_event :test_start, 0
        
        def test_start
          # Need to manually enable discovery
          # since link_status_updated is not defined.
          send_enable_topology_discovery
        end
        
        oneshot_timer_event :show_topology, 4
        
        def show_topology
          get_all_switch_status
          get_all_port_status
          get_all_link_status
        end
        
        def all_switch_status switches
          switches.each { |each| info Switch.new( each ).to_s }
        end
        
        def all_port_status ports
          ports.each { |each| info Port.new( each ).to_s }
        end
        
        def all_link_status links
          links.each { |each| info Link.new( each ).to_s }
        end
      end
      """
    When I successfully run `trema run GetAllTestController.rb -c GetAllTestController.conf -d`
    And wait until "topology" is up
    And *** sleep 4 ***
    Then the file "../../tmp/log/GetAllTestController.log" should contain:
      """
      Switch: 0x1 - {up:true}
      """
    And the file "../../tmp/log/GetAllTestController.log" should contain:
      """
      Switch: 0x2 - {up:true}
      """
    And the file "../../tmp/log/GetAllTestController.log" should match /Port: 0x1:1 - \{external:false, mac:"[0-9a-f:]+", name:".*", up:true\}/
    And the file "../../tmp/log/GetAllTestController.log" should match /Port: 0x2:1 - \{external:false, mac:"[0-9a-f:]+", name:".*", up:true\}/
    And the file "../../tmp/log/GetAllTestController.log" should contain:
      """
      Link: (0x1:1)->(0x2:1) - {unstable:false, up:true}
      """
    And the file "../../tmp/log/GetAllTestController.log" should contain:
      """
      Link: (0x2:1)->(0x1:1) - {unstable:false, up:true}
      """

  Scenario: Link discovery is automatically started if a link event handler is defined
    Given a file named "AutoEnableDiscovery.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      """
    And a file named "AutoEnableDiscovery.rb" with:
      """ruby
      class AutoEnableDiscovery < Controller
        include Topology
      
        def link_status_updated link_stat
          # define link event handler to implicitly enable link discovery
        end
      end
      """
    When I successfully run `trema run AutoEnableDiscovery.rb -c AutoEnableDiscovery.conf -d`
    And wait until "topology" is up
    Then the file "../../tmp/log/topology.log" should contain "Enabling topology discovery."

  Scenario: Link discovery can be manually started by send_enable_topology_discovery call
    Given a file named "EnableDiscovery.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      """
    And a file named "EnableDiscovery.rb" with:
      """ruby
      class EnableDiscovery < Controller
        include Topology
      
        oneshot_timer_event :test_start, 0
        
        def test_start
          send_enable_topology_discovery
        end
      end
      """
    When I successfully run `trema run EnableDiscovery.rb -c EnableDiscovery.conf -d`
    And wait until "topology" is up
    Then the file "../../tmp/log/topology.log" should contain "Enabling topology discovery."

  Scenario: Controller will automatically subscribe to topology if a topology event handler is defined
    Given a file named "SubscribeTestTopology.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      """
    And a file named "SubscribeTestTopology.rb" with:
      """ruby
      class SubscribeTestTopology < Controller
        include Topology
      
        def subscribe_topology_reply
          info "subscribe_topology_reply is called"
          info "subscribed? is true" if subscribed?
        end
        
        def link_status_updated link_attrs
          # Define topology event handler to automatically subscribe.
        end
      end
      """
    When I successfully run `trema run SubscribeTestTopology.rb -c SubscribeTestTopology.conf -d`
    And wait until "topology" is up
    Then the file "../../tmp/log/SubscribeTestTopology.log" should contain "subscribe_topology_reply is called"
    And the file "../../tmp/log/SubscribeTestTopology.log" should contain "subscribed? is true"

  Scenario: Controller will not automatically subscribe to topology if no topology event handler is defined
    Given a file named "NoSubscribeTestTopology.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      """
    And a file named "NoSubscribeTestTopology.rb" with:
      """ruby
      class NoSubscribeTestTopology < Controller
        include Topology
        
        oneshot_timer_event :test_start, 0
        def test_start
          info "subscribed? is false" if not subscribed?
        end
      end
      """
    When I successfully run `trema run NoSubscribeTestTopology.rb -c NoSubscribeTestTopology.conf -d`
    And wait until "topology" is up
    Then the file "../../tmp/log/NoSubscribeTestTopology.log" should contain "subscribed? is false"

  Scenario: Controller will not receive topology events after manually unsubscribing
    Given a file named "UnsubscribeTestTopology.rb" with:
      """ruby
      class UnsubscribeTestTopology < Controller
        include Topology
        
        def subscribe_topology_reply
          send_unsubscribe_topology do
            info "subscribed? is true" if subscribed?
          end
        end
        
        def switch_status_updated sw_attrs
          info "switch_status_updated is called"
        end
      end
      """
    And a file named "UnsubscribeTestTopology.conf" with:
      """
      vswitch("topology1") { datapath_id "0x1" }
      """
    When I successfully run `trema run UnsubscribeTestTopology.rb -d`
    And wait until "topology" is up
    And *** sleep 2 ***
    When I successfully run `trema run -c UnsubscribeTestTopology.conf -d`
    Then the file "../../tmp/log/UnsubscribeTestTopology.log" should not contain "subscribed? is true"
    Then the file "../../tmp/log/UnsubscribeTestTopology.log" should not contain "switch_status_updated is called"
