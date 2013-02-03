Feature: version command

  In order to know the current version of Trema
  As a developer using Trema
  I want to execute "trema version" command

  Scenario: trema version
    When I run `trema version`
    Then the output should match /trema version \d+\.\d+.\d+/
