Feature: Send a features request message

  As a Trema user
  I want to send a features request message to openflow switches
  So that I can get the list of switch features


  Scenario: Send a features request 
    When I try trema run "./objects/examples/openflow_message/features_request" with following configuration (backgrounded):
      """
      vswitch( "features_request" ) { datapath_id "0xabc" }
      """
      And wait until "features_request" is up
      And I terminated all trema services
    Then the output should include:
    """
    datapath_id: 0xabc
    n_buffers: 256
    n_tables: 1
    capabilities:
      OFPC_FLOW_STATS
      OFPC_TABLE_STATS
      OFPC_PORT_STATS
      OFPC_ARP_MATCH_IP
    actions:
      OFPAT_OUTPUT
      OFPAT_SET_VLAN_VID
      OFPAT_SET_VLAN_PCP
      OFPAT_STRIP_VLAN
      OFPAT_SET_DL_SRC
      OFPAT_SET_DL_DST
      OFPAT_SET_NW_SRC
      OFPAT_SET_NW_DST
      OFPAT_SET_NW_TOS
      OFPAT_SET_TP_SRC
      OFPAT_SET_TP_DST
    port_no: 65534
      name = vsw_0xabc
      config = 0x1
      state = 0x1
      curr = 0x82
      advertised = 0
      supported = 0
      peer = 0
    """

  Scenario: Send a features request in Ruby 
    When I try trema run "./src/examples/openflow_message/features-request.rb" with following configuration (backgrounded):
      """
      vswitch( "features-request" ) { datapath_id "0xabc" }
      """
      And wait until "FeaturesRequestController" is up
      And I terminated all trema services
    Then the output should include:
    """
    datapath_id: 0xabc
    n_buffers: 256
    n_tables: 1
    capabilities:
      OFPC_FLOW_STATS
      OFPC_TABLE_STATS
      OFPC_PORT_STATS
      OFPC_ARP_MATCH_IP
    actions:
      OFPAT_OUTPUT
      OFPAT_SET_VLAN_VID
      OFPAT_SET_VLAN_PCP
      OFPAT_STRIP_VLAN
      OFPAT_SET_DL_SRC
      OFPAT_SET_DL_DST
      OFPAT_SET_NW_SRC
      OFPAT_SET_NW_DST
      OFPAT_SET_NW_TOS
      OFPAT_SET_TP_SRC
      OFPAT_SET_TP_DST
    port_no: 65534
      name = vsw_0xabc
      config = 0x1
      state = 0x1
      curr = 0x82
      advertised = 0x0
      supported = 0x0
      peer = 0x0
    """
