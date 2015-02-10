# encoding: utf-8
begin
  require 'cucumber/rake/task'

  task :features => :build_trema
  Cucumber::Rake::Task.new(:features) do |t|
    t.cucumber_opts = '--tags @critical --tags ~@wip'
  end

  task 'features:all' => :build_trema
  Cucumber::Rake::Task.new('features:all') do |t|
    t.cucumber_opts = '--tags ~@wip'
  end
rescue LoadError
  task :features do
    $stderr.puts 'Cucumber is disabled'
  end
  task 'features:all' do
    $stderr.puts 'Cucumber is disabled'
  end
end
