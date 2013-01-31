Feature: --version option

  Use the --version option to display the current runtime version.

  Scenario: trema --version
    When I run `trema --version`
    Then the output should match /trema version \d+\.\d+.\d+/
