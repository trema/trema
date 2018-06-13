# frozen_string_literal: true

begin
  require 'reek/rake/task'
  Reek::Rake::Task.new do |t|
    t.fail_on_error = false
    t.verbose = false
  end
rescue LoadError
  task :reek do
    warn 'Reek is disabled'
  end
end
