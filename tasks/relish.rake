# frozen_string_literal: true

desc 'Push features to relish'
task 'relish:push' do
  if Kernel.const_defined?(:RELISH_PROJECT)
    sh "relish push #{RELISH_PROJECT}"
  else
    warn 'Relish is disabled'
  end
end
