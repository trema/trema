Feature: Packet-In filter configuration example

  As a Trema user
  I want to configure packetin_filter
  So that I can configuration filters of packetin_filter

  @wip
  Scenario: add filter
    When I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      event :port_status => "dumper", :packet_in => "filter", :state_notify => "dumper"
      filter :lldp => "dumper", :packet_in => "dumper"
      """
      And wait until "dumper" is up
      And *** sleep 2 ***
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/add_filter"
    Then the output should include:
      """
      A packetin filter was added ( match = [wildcards = 0xc(dl_src|dl_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 10, nw_src = 10.0.0.1/32, nw_dst = 10.0.0.2/32, tp_src = 1024, tp_dst = 2048], service_name = dumper ).
      """

  @wip
  Scenario: dump filter
    When I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      event :port_status => "dumper", :packet_in => "filter", :state_notify => "dumper"
      filter :lldp => "dumper", :packet_in => "dumper"
      """
      And wait until "dumper" is up
      And *** sleep 2 ***
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/dump_filter"
    Then the output should include:
      """
      2 packetin filters found ( match = [wildcards = 0x3fffff(all), in_port = 0, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0], service_name = dumper, strict = false ).
      [#0] match = [wildcards = 0x3fffef(in_port|dl_src|dl_dst|dl_vlan|dl_vlan_pcp|nw_proto|nw_tos|nw_src(63)|nw_dst(63)|tp_src|tp_dst), in_port = 0, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0x88cc, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0], priority = 32768, service_name = dumper.
      [#1] match = [wildcards = 0x3fffff(all), in_port = 0, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0], priority = 0, service_name = dumper.
      """

  @wip
  Scenario: dump filter strict
    When I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      event :port_status => "dumper", :packet_in => "filter", :state_notify => "dumper"
      filter :lldp => "dumper", :packet_in => "dumper"
      """
      And wait until "dumper" is up
      And *** sleep 2 ***
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/dump_filter_strict"
    Then the output should include:
      """
      0 packetin filter found ( match = [wildcards = 0xc(dl_src|dl_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 10, nw_src = 10.0.0.1/32, nw_dst = 10.0.0.2/32, tp_src = 1024, tp_dst = 2048], service_name = dumper, strict = true ).
      """
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/add_filter"
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/dump_filter_strict"
    Then the output should include:
      """
      1 packetin filter found ( match = [wildcards = 0xc(dl_src|dl_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 10, nw_src = 10.0.0.1/32, nw_dst = 10.0.0.2/32, tp_src = 1024, tp_dst = 2048], service_name = dumper, strict = true ).
      [#0] match = [wildcards = 0xc(dl_src|dl_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 10, nw_src = 10.0.0.1/32, nw_dst = 10.0.0.2/32, tp_src = 1024, tp_dst = 2048], priority = 65535, service_name = dumper.
      """

  @wip
  Scenario: delete filter strict
    When I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      event :port_status => "dumper", :packet_in => "filter", :state_notify => "dumper"
      filter :lldp => "dumper", :packet_in => "dumper"
      """
      And wait until "dumper" is up
      And *** sleep 2 ***
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/delete_filter_strict"
    Then the output should include:
      """
      0 packetin filter was deleted ( match = [wildcards = 0xc(dl_src|dl_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 10, nw_src = 10.0.0.1/32, nw_dst = 10.0.0.2/32, tp_src = 1024, tp_dst = 2048], service_name = dumper, strict = true ).
      """
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/add_filter"
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/delete_filter_strict"
    Then the output should include:
      """
      1 packetin filter was deleted ( match = [wildcards = 0xc(dl_src|dl_dst), in_port = 1, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0xffff, dl_vlan_pcp = 0, dl_type = 0x800, nw_tos = 0, nw_proto = 10, nw_src = 10.0.0.1/32, nw_dst = 10.0.0.2/32, tp_src = 1024, tp_dst = 2048], service_name = dumper, strict = true ).
      """

  @wip
  Scenario: delete filter
    When I try trema run "./objects/examples/dumper/dumper" with following configuration (backgrounded):
      """
      event :port_status => "dumper", :packet_in => "filter", :state_notify => "dumper"
      filter :lldp => "dumper", :packet_in => "dumper"
      """
      And wait until "dumper" is up
      And *** sleep 2 ***
      And I try to run "TREMA_HOME=`pwd` ./objects/examples/packetin_filter_config/delete_filter"
    Then the output should include:
      """
      2 packetin filters were deleted ( match = [wildcards = 0x3fffff(all), in_port = 0, dl_src = 00:00:00:00:00:00, dl_dst = 00:00:00:00:00:00, dl_vlan = 0, dl_vlan_pcp = 0, dl_type = 0, nw_tos = 0, nw_proto = 0, nw_src = 0.0.0.0/0, nw_dst = 0.0.0.0/0, tp_src = 0, tp_dst = 0], service_name = dumper, strict = false ).
      """
