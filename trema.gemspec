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
  gem.homepage = 'http://github.com/trema/trema_ruby'

  gem.executables = %w(trema)
  gem.files = `git ls-files`.split("\n")

  gem.test_files = `git ls-files -- {spec,features}/*`.split("\n")

  gem.add_dependency 'bundler', '~> 1.9.1'
  gem.add_dependency 'gli', '~> 2.13.0'
  gem.add_dependency 'phut', '~> 0.4.0'
  gem.add_dependency 'pio', '~> 0.18.2'
  gem.add_dependency 'rake'

  # Docs
  gem.add_development_dependency 'yard', '~> 0.8.7.6'

  # Test
  gem.add_development_dependency 'aruba', '~> 0.6.2'
  gem.add_development_dependency 'codeclimate-test-reporter', '~> 0.4.7'
  gem.add_development_dependency 'coveralls', '~> 0.7.11'
  gem.add_development_dependency 'cucumber', '~> 1.3.19'
  gem.add_development_dependency 'reek', '~> 2.0.2'
  gem.add_development_dependency 'rspec', '~> 3.2.0'
  gem.add_development_dependency 'rspec-given', '~> 3.7.0'
  gem.add_development_dependency 'rubocop', '~> 0.29.1'
end
