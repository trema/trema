Feature: netns
  Background:
    Given a file named "simple_hub.rb" with:
      """ruby
      class SimpleHub < Trema::Controller
        def switch_ready(dpid)
          send_flow_mod_add(
            dpid,
            match: Match.new,
            actions: SendOutPort.new(:flood)
          )
        end
      end
      """
    And a file named "simple_hub.conf" with:
      """ruby
      vswitch('simple_hub') { dpid 0x1 }
      netns('host1') {
        ip '192.168.1.2'
        netmask '255.255.255.0'
        route net: '0.0.0.0', gateway: '192.168.1.1'
      }
      netns('host2') {
        ip '192.168.1.3'
        netmask '255.255.255.0'
        route net: '0.0.0.0', gateway: '192.168.1.1'
      }
      link 'simple_hub', 'host1'
      link 'simple_hub', 'host2'
      """

  @sudo
  Scenario: netns namespece
    When I run `trema run simple_hub.rb -c simple_hub.conf -d`
    And I run `trema netns host1` interactively
    And I type "ip addr"
    And I type "exit"
    Then the stdout should contain "192.168.1.2"
