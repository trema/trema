Feature: trema-config compile helper

  As a Trema user
  I want to compile trema apps like: gcc -o hello_trema hello_trema.c `trema-config --cflags --libs`
  So that I can compile trema apps without gcc flags and options.

  Scenario: trema-config --cflags
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config --cflags`"
    Then the output should be:
      """
      """

  Scenario: trema-config -c
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config -c`"
    Then the output should be:
      """
      """

  Scenario: trema-config --libs
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config --cflags`"
     And I try to run "gcc hello_trema.o `./trema-config --libs`"
    Then the output should be:
      """
      """

  Scenario: trema-config -l
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config --cflags`"
     And I try to run "gcc hello_trema.o `./trema-config -l`"
    Then the output should be:
      """
      """

  Scenario: trema-config --cflags --libs
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config -c`"
     And I try to run "gcc hello_trema.o `./trema-config --cflags --libs`"
    Then the output should be:
      """
      """

  Scenario: trema-config -c --libs
    When I try to run "gcc src/examples/hello_trema/hello_trema.c `./trema-config -c --libs`"
    Then the output should be:
      """
      """

  Scenario: trema-config --cflags -l
    When I try to run "gcc src/examples/hello_trema/hello_trema.c `./trema-config --cflags -l`"
    Then the output should be:
      """
      """

  Scenario: trema-config -c -l
    When I try to run "gcc src/examples/hello_trema/hello_trema.c `./trema-config -c -l`"
    Then the output should be:
      """
      """

  Scenario: trema-config --help
    When I try to run "./trema-config --help"
    Then the output should be:
      """
      Usage: ./trema-config [OPTIONS ...]
          -c, --cflags
          -l, --libs
      """

  Scenario: trema-config -h
    When I try to run "./trema-config -h"
    Then the output should be:
      """
      Usage: ./trema-config [OPTIONS ...]
          -c, --cflags
          -l, --libs
      """
