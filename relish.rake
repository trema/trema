desc 'Push features to relish'
task 'relish:push' do
  if Kernel.const_defined?(:RELISH_PROJECT)
    sh "relish push #{RELISH_PROJECT}"
  else
    $stderr.puts 'Relish is disabled'
  end
end
