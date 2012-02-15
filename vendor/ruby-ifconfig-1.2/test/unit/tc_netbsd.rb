require File.dirname(__FILE__) + '/../test_helper'

class TC_NetBSDTest < Test::Unit::TestCase
  def setup
    sample = IO.readlines("#{File.dirname(__FILE__)}"+
                          '/../../ifconfig_examples/netbsd.txt').join
    @cfg = IfconfigWrapper.new('BSD',sample).parse
  end
  def test_interface_list
    assert(@cfg.interfaces == ["cs0", "lo0"],
           "Failed to parse all interfaces")
  end

  def test_mac_parse
    assert(@cfg['cs0'].mac == "08:00:2b:81:62:ca",
    "Failed to parse MAC address: "+@cfg['cs0'].mac)
  end

  def test_flags
    assert(@cfg['cs0'].flags.include?('BROADCAST') &&
          @cfg['cs0'].flags.include?('RUNNING') &&
          @cfg['cs0'].flags.include?('MULTICAST') &&
          @cfg['cs0'].up?,
           "FLAG Parsing failed: #{@cfg['cs0'].flags}")
  end

  def test_addr_types
    assert(@cfg['cs0'].addr_types.include?('inet') &&
           @cfg['cs0'].addr_types.include?('inet6'),
           "Failed to parse all address types")
  end

  def test_attribs
    assert(@cfg['cs0'].rx['bytes'].class == Fixnum || NilClass &&
           @cfg['cs0'].tx['bytes'].class == Fixnum || NilClass, "Wrong class")

  end

end
