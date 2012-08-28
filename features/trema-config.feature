Feature: trema-config command

  In order to specify C compiler options easily
  As a developer using Trema
  I want to use trema-config command

  Scenario: trema-config --cflags
    When I run `trema-config --cflags`
    Then the output should match /^-I\S+lib -I\S+openflow$/

  Scenario: trema-config -c
    When I run `trema-config -c`
    Then the output should match /^-I\S+lib -I\S+openflow$/

  Scenario: trema-config --libs
    When I run `trema-config --libs`
    Then the output should match /^-L\S+lib -l.*/

  Scenario: trema-config -l
    When I run `trema-config -l`
    Then the output should match /^-L\S+lib -l.*/

  Scenario: trema-config --cflags --libs
    When I run `trema-config --cflags --libs`
    Then the output should match /^-I\S+lib -I\S+openflow -L\S+lib -l.*/

  Scenario: trema-config -c --libs
    When I run `trema-config -c --libs`
    Then the output should match /^-I\S+lib -I\S+openflow -L\S+lib -l.*/

  Scenario: trema-config --cflags -l
    When I run `trema-config --cflags -l`
    Then the output should match /^-I\S+lib -I\S+openflow -L\S+lib -l.*/

  Scenario: trema-config -c -l
    When I run `trema-config -c -l`
    Then the output should match /^-I\S+lib -I\S+openflow -L\S+lib -l.*/

  Scenario: trema-config --help
    When I run `trema-config --help`
    Then the output should contain:
      """
      Usage: trema-config [OPTIONS ...]
          -c, --cflags
          -l, --libs
      """

  Scenario: trema-config -h
    When I run `trema-config -h`
    Then the output should contain:
      """
      Usage: trema-config [OPTIONS ...]
          -c, --cflags
          -l, --libs
      """
