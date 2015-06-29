begin
  require 'cucumber/rake/task'
  Cucumber::Rake::Task.new
  Cucumber::Rake::Task.new('cucumber:travis') do |task|
    task.cucumber_opts = '--tags ~@wip --tags ~@sudo --tags ~@shell'
  end
rescue LoadError
  task :cucumber do
    $stderr.puts 'Cucumber is disabled'
  end
  task 'cucumber:travis' do
    $stderr.puts 'Cucumber is disabled'
  end
end
