# encoding: utf-8
begin
  require 'rubocop/rake_task'
  RuboCop::RakeTask.new do |task| 
    task.fail_on_error = false
  end
rescue LoadError
  task :rubocop do
    $stderr.puts 'Rubocop is disabled'
  end
end
