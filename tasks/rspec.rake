begin
  require 'rspec/core'
  require 'rspec/core/rake_task'

  task :spec => :build_trema
  RSpec::Core::RakeTask.new do |task|
    task.verbose = $trace
    task.pattern = FileList['spec/**/*_spec.rb']
  end

  task 'spec:actions' => :build_trema
  RSpec::Core::RakeTask.new('spec:actions') do |task|
    task.verbose = $trace
    task.pattern = FileList['spec/**/*_spec.rb']
    task.rspec_opts = '--tag type:actions'
  end

  task 'spec:travis' => :build_trema
  RSpec::Core::RakeTask.new('spec:travis') do |task|
    task.verbose = $trace
    task.pattern = FileList['spec/trema/hello_spec.rb', 'spec/trema/echo-*_spec.rb']
    task.rspec_opts = '--tag ~sudo'
  end
rescue LoadError
  task :spec do
    $stderr.puts 'RSpec is disabled'
  end

  task 'spec:actions' do
    $stderr.puts 'RSpec is disabled'
  end

  task 'spec:travis' do
    $stderr.puts 'RSpec is disabled'
  end
end
