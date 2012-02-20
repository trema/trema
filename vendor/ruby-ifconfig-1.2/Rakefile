require 'rake'
require 'rake/testtask'

$VERBOSE = true

desc "Run all unit tests"
task :default => [ :test_units ]

desc "Run the unit tests in test/"
task :test_units do
  Dir.glob('test/unit/*').each do |t|
    puts `/usr/bin/env ruby #{t}`
  end
end
