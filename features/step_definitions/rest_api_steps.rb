begin
  require 'rack/test'

  World(Rack::Test::Methods)

  Given(/^I send and accept JSON$/) do
    header 'Accept', 'application/json'
    header 'Cotent-Type', 'application/json'
  end

  Given(/^I send a GET request for "([^\"]*)"$/) do |path|
    cd('.') { get path }
  end

  # rubocop:disable LineLength
  Given(/^I send a POST request for "([^\"]*)" with body "([^\"]*)"$/) do |path, body|
    cd('.') { post path, Object.instance_eval(body) }
  end
  # rubocop:enable LineLength

  # rubocop:disable LineLength
  Given(/^I send a DELETE request for "([^\"]*)" with body "([^\"]*)"$/) do |path, body|
    cd('.') { delete path, Object.instance_eval(body) }
  end
  # rubocop:enable LineLength

  Then(/^the response should be "([^\"]*)"$/) do |status|
    expect(last_response.status).to eq(status.to_i)
  end

  Then(/^the JSON response should be "([^\"]*)"$/) do |json|
    expect(JSON.parse(last_response.body)).to eq(JSON.parse(json))
  end

  Then(/^the JSON response should be:$/) do |json|
    expect(JSON.parse(last_response.body)).to eq(JSON.parse(json))
  end
rescue LoadError
  $stderr.puts 'Rack is disabled'
end
