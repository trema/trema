# A sample Guardfile
# More info at https://github.com/guard/guard#readme

guard :bundler do
  watch('Gemfile')
  watch(/^.+\.gemspec/)
end

guard :rubocop, all_on_start: false do
  watch(/.+\.rb$/)
  watch(/.+\.rake$/)
  watch(/(?:.+\/)?\.rubocop\.yml$/) { |m| File.dirname(m[0]) }
end

guard :rspec, cmd: 'bundle exec rspec --tag ~sudo' do
  watch(/^spec\/.+_spec\.rb$/)
  watch(%r{^ruby/trema/(.+)\.rb$}) { |m| "spec/trema/#{m[1]}_spec.rb" }
  # watch('spec/spec_helper.rb') { 'spec' }
end
