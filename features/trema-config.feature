Feature: trema-config compile helper

  As a Trema user
  I want to compile trema apps like: gcc -o hello_trema hello_trema.c `trema-config --cflags --libs`
  So that I can compile trema apps without specifying Trema-specific flags and options.


  Scenario: trema-config --cflags
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config --cflags` -o tmp/hello_trema.o"
    Then I should not get errors


  Scenario: trema-config -c
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config -c` -o tmp/hello_trema.o"
    Then I should not get errors


  Scenario: trema-config --libs
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config --cflags` -o tmp/hello_trema.o"
     And I try to run "gcc tmp/hello_trema.o `./trema-config --libs` -o tmp/hello_trema"
    Then I should not get errors


  Scenario: trema-config -l
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config --cflags` -o tmp/hello_trema.o"
     And I try to run "gcc tmp/hello_trema.o `./trema-config -l` -o tmp/hello_trema"
    Then I should not get errors


  Scenario: trema-config --cflags --libs
    When I try to run "gcc -c src/examples/hello_trema/hello_trema.c `./trema-config -c` -o tmp/hello_trema.o"
     And I try to run "gcc tmp/hello_trema.o `./trema-config --cflags --libs` -o tmp/hello_trema"
    Then I should not get errors


  Scenario: trema-config -c --libs
    When I try to run "gcc src/examples/hello_trema/hello_trema.c `./trema-config -c --libs` -o tmp/hello_trema"
    Then I should not get errors


  Scenario: trema-config --cflags -l
    When I try to run "gcc src/examples/hello_trema/hello_trema.c `./trema-config --cflags -l` -o tmp/hello_trema"
    Then I should not get errors


  Scenario: trema-config -c -l
    When I try to run "gcc src/examples/hello_trema/hello_trema.c `./trema-config -c -l` -o tmp/hello_trema"
    Then I should not get errors


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
