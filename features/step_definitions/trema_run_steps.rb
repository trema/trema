When(/^I trema run "([^"]*)"$/) do |controller_file|
  controller_path = if controller_file.include?('/')
                      File.join '..', '..', controller_file
                    else
                      controller_file
                    end
  step %(I run `trema run #{controller_path} -d`)
end

Given(/^I trema run "([^"]*)" with args "([^"]*)"$/) do |controller, args|
  step %(I run `trema run #{controller} #{args}`)
  step %(sleep 5)
end

Given(/^I successfully trema run "([^"]*)" with args "([^"]*)"$/) do |controller, args|
  step %(I successfully run `trema run #{controller} #{args}`)
  step %(sleep 5)
end

When(/^I trema run "([^"]*)" interactively$/) do |controller|
  step %(I run `trema run #{controller}` interactively)
  step %(sleep 2)
end

When(/^I trema "([^"]*)" run "([^"]*)" interactively$/) do |global_option, controller|
  step %(I run `trema #{global_option} run #{controller}` interactively)
  step %(sleep 2)
end

When(/^I trema run "([^"]*)" with args "([^"]*)" interactively$/) do |controller, args|
  step %(I run `trema run #{controller} #{args}` interactively)
  step %(sleep 2)
end

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
