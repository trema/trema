$:.push File.expand_path( "../ruby", __FILE__ )
require "trema/version"

Gem::Specification.new do | gem |
  gem.name = "trema"
  gem.version = Trema::VERSION
  gem.authors = [ "Yasuhito Takamiya" ]
  gem.email = [ "yasuhito@gmail.com" ]
  gem.homepage = "http://github.com/trema/trema"
  gem.summary = %q{Full-Stack OpenFlow Framework for Ruby and C}
  gem.description = %q{Trema is a full-stack, easy-to-use framework for developing OpenFlow controllers in Ruby and C}
  gem.license = "GPL2"
  gem.required_ruby_version = "~> 1.8.7"

  gem.files = `git ls-files`.split( "\n" )
  gem.test_files = `git ls-files -- {spec,features}/*`.split( "\n" )
  gem.bindir = "."
  gem.executables = [ "trema", "trema-config" ]
  gem.require_path = "ruby"
  gem.extensions = [ "Rakefile" ]
  gem.extra_rdoc_files = [ "README.md" ]

  # specify any dependencies here; for example:
  # gem.add_development_dependency "rspec"
  gem.add_runtime_dependency "rake"
  gem.add_runtime_dependency "gli", "~> 2.5.4"
  gem.add_runtime_dependency "rdoc", "~> 3.12"

  gem.add_development_dependency "rake"
  gem.add_development_dependency "rdoc"
  gem.add_development_dependency "aruba"
end


### Local variables:
### mode: Ruby
### coding: utf-8-unix
### indent-tabs-mode: nil
### End:
