$LOAD_PATH.push File.expand_path('../lib', __FILE__)
require 'trema/version'

Gem::Specification.new do |gem|
  gem.name = 'trema'
  gem.version = Trema::VERSION
  gem.summary = 'Full-stack OpenFlow framework.'
  gem.description = 'Trema is a full-stack, easy-to-use framework ' \
                    'for developing OpenFlow controllers in Ruby.'

  gem.required_ruby_version = '>= 2.0.0'

  gem.license = 'GPL3'

  gem.authors = ['Yasuhito Takamiya']
  gem.email = ['yasuhito@gmail.com']
  gem.homepage = 'http://github.com/trema/trema'

  gem.executables = %w(trema)
  gem.files = `git ls-files`.split("\n")

  gem.test_files = `git ls-files -- {spec,features}/*`.split("\n")

  gem.add_dependency 'bundler', '~> 1.11.2'
  gem.add_dependency 'gli', '~> 2.13.4'
  gem.add_dependency 'phut', '~> 0.7.7'
  gem.add_dependency 'pio', '~> 0.30.0'

  # Docs
  gem.add_development_dependency 'relish', '~> 0.7.1'
  gem.add_development_dependency 'yard', '~> 0.8.7.6'

  # Test
  gem.add_development_dependency 'aruba', '~> 0.13.0'
  gem.add_development_dependency 'codeclimate-test-reporter', '~> 0.4.8'
  gem.add_development_dependency 'coveralls', '~> 0.8.10'
  gem.add_development_dependency 'cucumber', '~> 2.3.2'
  gem.add_development_dependency 'rake'
  gem.add_development_dependency 'reek', '~> 3.10.2'
  gem.add_development_dependency 'rspec', '~> 3.4.0'
  gem.add_development_dependency 'rspec-given', '~> 3.8.0'
  gem.add_development_dependency 'rubocop', '~> 0.37.2'
end
