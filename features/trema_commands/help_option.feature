Feature: --help option

  Use the --help option or just type trema to display the usage of trema command.

  Scenario: trema --help
    When I run `trema --help`
    Then the output should match /^NAME/
     And the output should match /^SYNOPSIS/
     And the output should match /^VERSION/
     And the output should match /^GLOBAL OPTIONS/
     And the output should match /^COMMANDS/

  Scenario: trema
    When I run `trema`
    Then the output should match /^NAME/
     And the output should match /^SYNOPSIS/
     And the output should match /^VERSION/
     And the output should match /^GLOBAL OPTIONS/
     And the output should match /^COMMANDS/
