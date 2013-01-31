Feature: help command

  In order to understand how to use Trema
  As a developer using Trema
  I want to read the help message of trema command

  Scenario: List sub-commands
    When I run `trema help`
    Then the output should contain "dump_flows"
     And the output should contain "help"
     And the output should contain "kill"
     And the output should contain "killall"
     And the output should contain "netns"
     And the output should contain "reset_stats"
     And the output should contain "ruby"
     And the output should contain "run"
     And the output should contain "send_packets"
     And the output should contain "show_stats"
     And the output should contain "up"
     And the output should contain "version"

  Scenario: List global options
    When I run `trema help`
    Then the output should contain "--help"
     And the output should contain "-v, --verbose"
     And the output should contain "--version"
