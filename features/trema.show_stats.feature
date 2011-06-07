Feature: show network stats with `trema show_stats' command

  As a Trema user
  I want to investigate several network stats with `trema show_stats' command
  So that I can easily debug trema applications

  Scenario: trema help show_stats
    When I try to run "./trema help show_stats"
    Then the output should be:
      """
      Usage: ./trema show_stats [OPTIONS ...]
          -t, --tx
          -r, --rx

          -h, --help
          -v, --verbose
      """
