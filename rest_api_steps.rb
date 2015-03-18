require 'rack/test'
require 'routing_switch/api'

World(Rack::Test::Methods)

def app
  RoutingSwitch::Api
end

Given(/^I send and accept JSON$/) do
  header 'Accept', 'application/json'
  header 'Cotent-Type', 'application/json'
end

Given(/^I send a GET request for "([^\"]*)"$/) do |path|
  get path
end

Then(/^the response should be "([^\"]*)"$/) do |status|
  expect(last_response.status).to eq(status.to_i)
end

Then(/^the JSON response should be an empty array$/) do
  expect(JSON.parse(last_response.body)).to eq([])
end
