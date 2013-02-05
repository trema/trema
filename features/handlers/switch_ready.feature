Feature: switch_ready handler

  The switch-ready is a message indicating a successful connection
  established between a switch and your controller. If one wishes to
  detect the connection-establishment event one must implement the
  :switch_ready method in the controller class.

  Note that the switch-ready message is not part of the OpenFlow spec,
  but is a Trema-specific message. Internally, Trema::Controller
  executes the following steps to emulate the switch-ready message.

  1. Upon connection establishment with the switch a Hello message is
  sent that completes a version negotiation sequence.
  2. A Feature-Request message is sent awaiting for a Features-Reply
  message from the switch.
  3. Switch's state and flow tables are cleared/initialized to ensure
  further correct operation.
  4. Trema::Controller redirects a :switch_ready message to itself
  with the switch's datapath-ID as its argument.

  As Trema performs the above steps in the background, users are
  unaware of such details. Users only need to handle the switch-ready
  message.

  @slow_process
  Scenario: switch_ready handler
    Given a file named "switch-ready-detector.rb" with:
    """
    class SwitchReadyDetector < Controller
      def switch_ready datapath_id
        info "Switch #{ datapath_id.to_hex } is UP"
      end
    end
    """
    And a file named "sample.conf" with:
    """
    vswitch { datapath_id "0xabc" }
    """
    When I run `trema run ./switch-ready-detector.rb -c sample.conf` interactively
    Then the output should contain "Switch 0xabc is UP" within the timeout period
