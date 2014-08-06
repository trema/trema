@ruby
Feature: send_packets command

  In order to send/receive packets between virtual hosts
  As a developer using Trema
  I want to execute "trema send_packets" command

  Background:
    Given a file named "null_controller.rb" with:
    """
    class NullController < Controller; end
    """
    And a file named "network.conf" with:
    """
    vswitch { datapath_id 0xabc }

    vhost("host1")
    vhost("host2")

    link "0xabc", "host1"
    link "0xabc", "host2"
    """
    And I run `trema run null_controller.rb -c network.conf -d`

  @slow_process
  Scenario: unknown host error (--source)
    When I run `trema send_packets --source NO_SUCH_HOST --dest host2`
    Then the output should contain:
    """
    unknown host: NO_SUCH_HOST
    """

  @slow_process
  Scenario: unknown host error (--dest)
    When I run `trema send_packets --source host1 --dest NO_SUCH_HOST`
    Then the output should contain:
    """
    unknown host: NO_SUCH_HOST
    """
