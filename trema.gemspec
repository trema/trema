$:.push File.expand_path( "../ruby", __FILE__ )
require "trema/version"


Gem::Specification.new do | gem |
  gem.name = "trema"
  gem.version = Trema::VERSION
  gem.summary = "Full-stack OpenFlow framework."
  gem.description = "Trema is a full-stack, easy-to-use framework for developing OpenFlow controllers in Ruby and C."

  gem.required_ruby_version = "~> 1.8.7"

  gem.license = "GPL2"

  gem.authors = [ "Yasuhito Takamiya" ]
  gem.email = [ "yasuhito@gmail.com" ]
  gem.homepage = "http://github.com/trema/trema"

  gem.bindir = "."
  gem.executables = [ "trema", "trema-config" ]
  gem.files = `git ls-files`.split( "\n" )

  gem.require_path = "ruby"
  gem.extensions = [ "Rakefile" ]

  gem.extra_rdoc_files = [ "README.md" ]
  gem.test_files = `git ls-files -- {spec,features}/*`.split( "\n" )

  gem.add_dependency "bundler"
  gem.add_dependency "gli", "~> 2.5.5"
  gem.add_dependency "paper-house", "~> 0.1.13"
  gem.add_dependency "rake", "~> 10.0.3"
  gem.add_dependency "rdoc", "~> 4.0.0"
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
