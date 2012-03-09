Feature: trema help

  As a Trema user
  I want to see the help message of trema command
  So that I can learn how to use trema

  Scenario: trema help
    When I try to run "./trema help"
    Then the output should be:
      """
      usage: trema <COMMAND> [OPTIONS ...]

      Trema command-line tool
      Type 'trema help <COMMAND>' for help on a specific command.

      Available commands:
        run            - runs a trema application.
        kill           - terminates a trema process.
        up             - starts a killed trema process again.
        killall        - terminates all trema processes.
        send_packets   - sends UDP packets to destination host.
        show_stats     - shows stats of packets.
        reset_stats    - resets stats of packets.
        dump_flows     - print all flow entries.
        ruby           - opens in your browser Trema's Ruby API documentation.
        version        - displays the current runtime version.
      """
