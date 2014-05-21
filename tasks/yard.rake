# encoding: utf-8
begin
  require 'yard'
  YARD::Rake::YardocTask.new do |t|
    t.files = ['ruby/trema/**/*.c', 'ruby/trema/**/*.rb']
    t.options = ['--no-private']
    t.options << '--debug' << '--verbose' if verbose == true
  end
rescue LoadError
  task :yard do
    $stderr.puts 'YARD is disabled'
  end
end
