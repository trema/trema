Given(/^I use OpenFlow 1\.0$/) do
  @open_flow_version = :open_flow10
end

Given(/^I use OpenFlow 1\.3$/) do
  @open_flow_version = :open_flow13
end

Given(/^a socket directory named "([^"]*)"$/) do |socket_directory|
  step %(a directory named "#{socket_directory}")
  ENV['TREMA_SOCKET_DIR'] = socket_directory
end

Then(/^a socket file named "([^"]*)" should exist$/) do |socket_file|
  cd('.') do
    expect(FileTest.socket?(socket_file)).to be_truthy
  end
end

When(/^I trema killall "([^"]*)"$/) do |controller|
  step %(I successfully run `trema killall #{controller}`)
end

When(/^I delete the link between "([^"]*)" and "([^"]*)"$/) do |peer1, peer2|
  step %(I successfully run `trema delete_link #{peer1} #{peer2}`)
  step %(sleep 3)
end

Then(/^the log file "([^"]*)" should contain following messages:$/) do |log_file, messages|
  step %(a file named "#{log_file}" should exist)
  messages.rows.flatten.each do |each|
    step %(the file "#{log_file}" should contain "#{each}")
  end
end

Then(/^the command returns immediately$/) do
  # noop
end

When(/^sleep (\d+)$/) do |time|
  sleep time.to_i
end
