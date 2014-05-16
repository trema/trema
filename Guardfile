# A sample Guardfile
# More info at https://github.com/guard/guard#readme

guard :bundler do
  watch('Gemfile')
  # Uncomment next line if your Gemfile contains the `gemspec' command.
  watch(/^.+\.gemspec/)
end

guard :rubocop do
  watch(/.+\.rb$/)
  watch(/.+\.rake$/)
  watch(/(?:.+\/)?\.rubocop\.yml$/) { |m| File.dirname(m[0]) }
end

guard :rspec do
  watch(%r{^spec/.+_spec\.rb$})
  watch(%r{^ruby/trema/(.+)\.rb$})     { |m| "spec/trema/#{m[1]}_spec.rb" }
  watch('spec/spec_helper.rb')  { "spec" }
end

