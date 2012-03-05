Feature: reset network stats with `trema reset_stats' command

  As a Trema user
  I want to reset network stats with `trema reset_stats' command
  So that I can easily debug trema applications

  Scenario: trema help reset_stats
    When I try to run "./trema help reset_stats"
    Then the output should be:
      """
      Usage: trema reset_stats [OPTIONS ...]
          -h, --help
          -v, --verbose
      """
