Feature: Tests for get_trema_process_from_name, read_pid
  
  Following scenarios are test cases for
  read_pid(), which is called inside get_trema_process_from_name().

  Background: 
    Given a file named "get_pid_for_trema_process.c" with:
      """
      #include <stdio.h>
      #include "trema.h"
      
      int
      main( int argc, char* argv[] ) {
        init_trema( &argc, &argv );
        
        int i;
        for( i = 1 ; i < argc ; ++i ) {
          pid_t pid = get_trema_process_from_name( argv[ i ] );
          printf( "%s: %d\n", argv[ i ], pid );
        }
        start_trema_up();
        start_trema_down();
        return 0;
      }
      """
    And I compile "get_pid_for_trema_process.c" into "get_pid_for_trema_process"
    Given a file named "empty_c_controller.c" with:
      """
      #include "trema.h"
      
      int
      main( int argc, char* argv[] ) {
        init_trema( &argc, &argv );
        start_trema();
        return 0;
      }
      """

  Scenario: Normal Ruby Controller
    Given a file named "EmptyRubyController.rb" with:
      """
      class EmptyRubyController < Controller
      end
      """
    And I run `trema run ./EmptyRubyController.rb -d`
    And wait until "EmptyRubyController" is up
    When I run `trema run "./get_pid_for_trema_process EmptyRubyController"` interactively
    Then the output should match:
      """
      EmptyRubyController: \d+
      """

  Scenario: Renamed Ruby Controller
    Given a file named "RenamedRubyController.rb" with:
      """
      class RenamedRubyController < Controller
        def name
          "SomeName"
        end
      end
      """
    And I run `trema run "./RenamedRubyController.rb" -d`
    And wait until "SomeName" is up
    When I run `trema run "./get_pid_for_trema_process SomeName"` interactively
    Then the output should match:
      """
      SomeName: \d+
      """

  Scenario: Normal C Controller
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller" -d`
    And wait until "empty_c_controller" is up
    When I run `trema run "./get_pid_for_trema_process empty_c_controller"` interactively
    Then the output should match:
      """
      empty_c_controller: \d+
      """

  Scenario: Renamed C Controller (short)
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller -n SomeNameC" -d`
    And wait until "SomeNameC" is up
    When I run `trema run "./get_pid_for_trema_process SomeNameC"` interactively
    Then the output should match:
      """
      SomeNameC: \d+
      """

  Scenario: Renamed C Controller (long)
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller --name=AnotherNameC" -d`
    And wait until "AnotherNameC" is up
    When I run `trema run "./get_pid_for_trema_process AnotherNameC"` interactively
    Then the output should match:
      """
      AnotherNameC: \d+
      """

  Scenario: Invalid Controller
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller" -d`
    And wait until "empty_c_controller" is up
    When I run `trema run "./get_pid_for_trema_process Invalid"` interactively
    Then the output should match:
      """
      Invalid: -1
      """
