desc 'Enforce Ruby code quality with static analysis of code'
task :quality => [:reek, :flog, :flay, :rubocop]
