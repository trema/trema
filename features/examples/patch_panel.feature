Feature: "Patch Panel" sample application

  In order to learn how to implement software patch panel
  As a developer using Trema
  I want to execute "Patch Panel" sample application

  @slow_process
  Scenario: Run "Patch Panel" Ruby example
    Given a file named "patch-panel.conf" with:
      """
      1 2
      """
     And a file named "network.conf" with:
      """
      vswitch("patch") { datapath_id "0xabc" }

      vhost("host1") { ip "192.168.0.1" }
      vhost("host2") { ip "192.168.0.2" }

      link "patch", "host1"
      link "patch", "host2"
      """
     And I run `trema run ../../src/examples/patch_panel/patch-panel.rb -c network.conf -d`
     And wait until "PatchPanel" is up
    When I send 1 packet from host1 to host2
     And I run `trema show_stats host1 --tx`
     And I run `trema show_stats host2 --rx`
    Then the output from "trema show_stats host1 --tx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
     And the output from "trema show_stats host2 --rx" should contain "192.168.0.2,1,192.168.0.1,1,1,50"
