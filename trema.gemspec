# -*- encoding: utf-8 -*-
$:.push File.expand_path( "../ruby", __FILE__ )
require "trema/version"

Gem::Specification.new do | s |
  s.name = "trema"
  s.version = Trema::VERSION
  s.authors = [ "Yasuhito Takamiya" ]
  s.email = [ "yasuhito@gmail.com" ]
  s.homepage = "http://github.com/trema/trema"
  s.summary = %q{Full-Stack OpenFlow Framework for Ruby and C}
  s.description = %q{Trema is a full-stack, easy-to-use framework for developing OpenFlow controllers in Ruby and C}
  s.license = "GPL2"

  s.files = `git ls-files`.split( "\n" )
  s.test_files = `git ls-files -- {spec,features}/*`.split( "\n" )
  s.bindir = "."
  s.executables = [ "trema", "trema-config" ]
  s.require_path = "ruby"
  s.extensions = [ "Rakefile" ]
  s.extra_rdoc_files = [ "README.md" ]

  # specify any dependencies here; for example:
  # s.add_development_dependency "rspec"
  s.add_runtime_dependency "rake"
end
