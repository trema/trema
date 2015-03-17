begin
  require 'cucumber/rake/task'
  Cucumber::Rake::Task.new
rescue LoadError
  task :cucumber do
    $stderr.puts 'Cucumber is disabled'
  end
end
