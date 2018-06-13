# frozen_string_literal: true

begin
  require 'yard'
  YARD::Rake::YardocTask.new do |t|
    t.options = ['--no-private']
    t.options << '--debug' << '--verbose' if verbose == true
  end
rescue LoadError
  task :yard do
    warn 'YARD is disabled'
  end
end
