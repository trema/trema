Feature: send packets with `trema send_packets' command

  As a Trema user
  I want to send network packets with `trema send_packets' command
  So that I can easily debug trema applications

  Scenario: trema help send_packets
    When I try to run "./trema help send_packets"
    Then the output should be:
      """
      Usage: ./trema send_packets [OPTIONS ...]
          -s, --source HOSTNAME
              --inc_ip_src [NUMBER]
          -d, --dest HOSTNAME
              --inc_ip_dst [NUMBER]
              --tp_src NUMBER
              --inc_tp_src [NUMBER]
              --tp_dst NUMBER
              --inc_tp_dst [NUMBER]
              --pps NUMBER
              --n_pkts NUMBER
              --duration NUMBER
              --length NUMBER
              --inc_payload [NUMBER]

          -h, --help
          -v, --verbose
      """
