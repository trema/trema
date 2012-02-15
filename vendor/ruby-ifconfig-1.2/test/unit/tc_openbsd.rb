require File.dirname(__FILE__) + '/../test_helper'

class TC_OpenBSDTest < Test::Unit::TestCase
  def setup
    sample = IO.readlines("#{File.dirname(__FILE__)}"+
                          '/../../ifconfig_examples/openbsd.txt').join
    @cfg = IfconfigWrapper.new('BSD',sample).parse
  end
  def test_interface_list
    assert(@cfg.interfaces == ["lo0", "lo1", "ep0", "xl0"],
           "Failed to parse all interfaces")
  end

  def test_mac_parse
    assert(@cfg['xl0'].mac == "00:01:02:c6:4b:3d",
    "Failed to parse MAC address: "+@cfg['xl0'].mac)
  end

  def test_flags
    assert(@cfg['xl0'].flags.include?('BROADCAST') &&
          @cfg['xl0'].flags.include?('RUNNING') &&
          @cfg['xl0'].flags.include?('MULTICAST') &&
          @cfg['xl0'].up?,
           "FLAG Parsing failed: #{@cfg['xl0'].flags}")
  end

  def test_addr_types
    assert(@cfg['xl0'].addr_types.include?('inet') &&
           @cfg['xl0'].addr_types.include?('inet6'),
           "Failed to parse all address types")
  end

  def test_attribs
    assert(@cfg['xl0'].rx['bytes'].class == Fixnum || NilClass &&
           @cfg['xl0'].tx['bytes'].class == Fixnum || NilClass, "Wrong class")

  end

end
