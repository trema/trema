require File.dirname(__FILE__) + '/../test_helper'

class TC_Linux < Test::Unit::TestCase
  def setup
    sample = IO.readlines("#{File.dirname(__FILE__)}"+
                        '/../../ifconfig_examples/linux.txt').join
    @cfg = IfconfigWrapper.new('Linux',sample).parse
  end

  def test_intefaces
    assert(@cfg.interfaces == ["sl0", "lo", "ppp0", "eth0",
                                "eth0:0", "sit0", "sit1"],
           "Not all interfaces parsed")
  end

  def test_mtu
    assert(@cfg['eth0'].mtu == 1500, "Failed to parse eth0 mtu")
  end

  def test_mac_parse
    assert(@cfg['eth0'].mac == "00:50:DA:C1:C3:45",
    "Failed to parse MAC address: "+@cfg['eth0'].mac)
  end

  def test_flags
    types = ['BROADCAST', 'RUNNING', 'MULTICAST']
    types.each do |t|
      assert(@cfg['eth0'].flags.include?(t),
             "FLAG Parsing failed: #{@cfg['eth0'].flags}")
    end
    assert(@cfg['eth0'].up?,
           "FLAG Parsing failed: #{@cfg['eth0'].flags}")
  end

  def test_networks
    types = ['EtherTalk Phase 2', 'IPX/Ethernet 802.2', 'IPX/Ethernet 802.3',
    'IPX/Ethernet II','IPX/Ethernet SNAP','inet', 'inet6']
    types.each do |t|
      assert(@cfg['eth0'].addr_types.include?(t),
             "Missing Address Types: #{@cfg['eth0'].addr_types}")
    end
  end

  def test_attribs
    assert(@cfg['eth0'].rx['bytes'] == 1037052545)
    assert(@cfg['eth0'].tx['bytes'] == 32859376)
  end

end
