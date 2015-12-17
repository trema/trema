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

When(/^I trema run "([^"]*)"$/) do |controller_file|
  controller_path = if controller_file.include?('/')
                      File.join '..', '..', controller_file
                    else
                      controller_file
                    end
  step %(I run `trema run #{controller_path} -d`)
end

Given(/^I trema run "([^"]*)" with args "([^"]*)"$/) do |controller, args|
  controller_path = File.join('..', '..', controller)
  step %(I successfully run `trema run #{controller_path} #{args}`)
  step %(sleep 10)
end

When(/^I trema run "([^"]*)" interactively$/) do |controller_file|
  step %(I run `trema run #{controller_file}` interactively)
end

# rubocop:disable LineLength
When(/^I trema run "([^"]*)"( interactively)? with the configuration "([^"]*)"$/) do |controller_file, interactive, configuration_file|
  open_flow_option = @open_flow_version == :open_flow13 ? ' --openflow13' : ''
  controller_path = if controller_file.include?('/')
                      File.join '..', '..', controller_file
                    else
                      controller_file
                    end
  run_arguments = "#{controller_path}#{open_flow_option} -c #{configuration_file}"
  if interactive
    step %(I run `trema run #{run_arguments}` interactively)
  else
    step %(I successfully run `trema run #{run_arguments} -d`)
  end
  step %(sleep 10)
end
# rubocop:enable LineLength

When(/^I trema killall "([^"]*)"$/) do |controller_name|
  step %(I successfully run `trema killall #{controller_name}`)
end

When(/^I delete the link between "([^"]*)" and "([^"]*)"$/) do |peer1, peer2|
  step %(I successfully run `trema delete_link #{peer1} #{peer2}`)
  step %(sleep 3)
end

# rubocop:disable LineLength
Then(/^the log file "([^"]*)" should contain following messages:$/) do |log_file, messages|
  step %(a file named "#{log_file}" should exist)
  messages.rows.flatten.each do |each|
    step %(the file "#{log_file}" should contain "#{each}")
  end
end
# rubocop:enable LineLength

Then(/^the command returns immediately$/) do
  # noop
end

When(/^sleep (\d+)$/) do |time|
  sleep time.to_i
end
