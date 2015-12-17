require 'simplecov'
require 'codeclimate-test-reporter'
require 'coveralls'

formatters = [SimpleCov::Formatter::HTMLFormatter]
formatters << Coveralls::SimpleCov::Formatter if ENV['COVERALLS_REPO_TOKEN']
if ENV['CODECLIMATE_REPO_TOKEN']
  formatters << CodeClimate::TestReporter::Formatter
end

SimpleCov.formatter = SimpleCov::Formatter::MultiFormatter.new(formatters)
SimpleCov.start { add_filter '/vendor/' }

require 'rspec/given'
