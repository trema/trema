RSpec::Matchers.define :have_constant do | const, klass |
  match do | owner |
    const_defined = owner.const_defined?( const )
    klass_match = owner.const_get( const ).class == klass unless klass.nil?
    klass.nil? ? const_defined : ( const_defined && klass_match )
  end

  failure_message_for_should do | actual |
    msg = "constant #{ expected[ 0 ] } not defined in #{ actual }"
    msg += " as a #{ expected[ 1 ] }" unless expected[ 1 ].nil?
    msg
  end

  failure_message_for_should_not do | actual |
    msg = "constant #{ expected[ 0 ] } is defined in #{ actual }"
    msg += " as a #{ expected[ 1 ] }" unless expected[ 1 ].nil?
    msg
  end

  description do
    msg = "have constant #{ const }"
    msg += " defined with class #{ klass }" unless klass.nil?
    msg
  end
end
