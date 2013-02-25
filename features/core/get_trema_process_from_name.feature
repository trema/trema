Feature: get_trema_process_from_name
  
      \#include "trema.h"
      
      pid_t get_trema_process_from_name( const char *name )
  
  Trema C API get_trema_process_from_name() returns the pid of the trema process 
  or it returns -1 if the specified trema process did not exist. 
  
  The `name` argument is a name within trema's name space and it may not always 
  match the process name in the Operating System.  
  For example if the trema process is a Ruby Controller, 
  it may be the Ruby class name of the Controller class, 
  or it may be the name defined as a return value of Controller's `#name` method.  
  If the trema process is a C Controller, 
  it may be the executable name of the C Controller, 
  or it may be the name specified by trema command line option `-n` or `--name`.  

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
          printf( "PID of %s = %d\n", argv[ i ], pid );
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

  Scenario: returns the pid for Ruby Controller with default naming.(=Class Name)
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
      PID of EmptyRubyController = \d+
      """

  Scenario: returns the pid for Ruby Controller with explicitly defined name.
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
      PID of SomeName = \d+
      """

  Scenario: returns the pid of C Controller with default naming.(=executable name)
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller" -d`
    And wait until "empty_c_controller" is up
    When I run `trema run "./get_pid_for_trema_process empty_c_controller"` interactively
    Then the output should match:
      """
      PID of empty_c_controller = \d+
      """

  Scenario: returns the pid of C Controller renamed through CLI option '-n'
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller -n SomeNameC" -d`
    And wait until "SomeNameC" is up
    When I run `trema run "./get_pid_for_trema_process SomeNameC"` interactively
    Then the output should match:
      """
      PID of SomeNameC = \d+
      """

  Scenario: return the pid of C Controller renamed through CLI option '--name'
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller --name=AnotherNameC" -d`
    And wait until "AnotherNameC" is up
    When I run `trema run "./get_pid_for_trema_process AnotherNameC"` interactively
    Then the output should match:
      """
      PID of AnotherNameC = \d+
      """

  Scenario: returns -1 when specfied controller name does not exists.
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller" -d`
    And wait until "empty_c_controller" is up
    When I run `trema run "./get_pid_for_trema_process Invalid"` interactively
    Then the output should match:
      """
      PID of Invalid = -1
      """
