# encoding: utf-8
source 'https://rubygems.org'

# Include dependencies from trema.gemspec. DRY!
gemspec

# Add dependencies required to use your gem here.

group :docs do
  gem 'relish', '~> 0.7'
  gem 'yard', '~> 0.8.7.4'
end

group :development do
  gem 'byebug', '~> 3.1.2', platforms: :ruby_20
  gem 'guard', '~> 2.6.1'
  gem 'guard-bundler', '~> 2.0.0'
  gem 'guard-rspec', '~> 4.2.9'
  gem 'guard-rubocop', '~> 1.1.0'
  gem 'pry', '~> 0.9.12.6'
end

group :test do
  gem 'aruba', '~> 0.5.4'
  gem 'codeclimate-test-reporter', require: nil
  gem 'cucumber', '~> 1.3.15'
  gem 'flay', '~> 2.4.0'
  gem 'flog', '~> 4.2.0'
  gem 'fuubar', '~> 1.3.3'
  gem 'reek', '~> 1.3.7'
  gem 'rspec', '~> 2.14.1'
  gem 'rubocop', '~> 0.22.0', platforms: [:ruby_19, :ruby_20, :ruby_21]
end
