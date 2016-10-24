source 'https://rubygems.org'

gemspec development_group: :test

git_source(:github) do |repo_name|
  repo_name = "#{repo_name}/#{repo_name}" unless repo_name.include?('/')
  "https://github.com/#{repo_name}.git"
end

gem 'pio', github: 'trema/pio', branch: 'develop'
gem 'phut', github: 'trema/phut', branch: 'develop'
