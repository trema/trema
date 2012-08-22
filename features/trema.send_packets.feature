Feature: send packets with `trema send_packets' command

  As a Trema user
  I want to send network packets with `trema send_packets' command
  So that I can easily debug trema applications


  Background:
    Given I try trema run "learning_switch" example with following configuration (backgrounded):
      """
      vswitch { datapath_id "0xabc" }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "0xabc", "host1"
      link "0xabc", "host2"
      """


  Scenario: send_packets error (--source)
    Then "./trema send_packets --source NO_SUCH_HOST --dest host2" exits abnormally with an error message:
      """
      error: Unknown host: NO_SUCH_HOST
      """


  Scenario: send_packets error (--dest)
    Then "./trema send_packets --source host1 --dest NO_SUCH_HOST" exits abnormally with an error message:
      """
      error: Unknown host: NO_SUCH_HOST
      """
