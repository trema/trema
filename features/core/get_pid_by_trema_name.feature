Feature: get_pid_by_trema_name()
  
  The function get_pid_by_trema_name() returns the PID of the process 
  specified by its `name` argument. 
  If the `name` is not found, the function returns -1. 
  
      pid_t get_pid_by_trema_name( const char *name )
  
  The `name` argument refers to controller process name called *trema-name*. 
  In the trema framework the trema-name is a unique identifier assigned to 
  each controller process. 
  It is determined automatically by the following rules: 
  
  **Rule 1:** When you start a Ruby controller:
  
      class TopologyController < Controller
        ...
      end
  
      $ trema run topology-controller.rb
  
  then its trema-name is set to "TopologyController", 
  which is the same as the class name.  
  
  **Rule 2:** When you start a C controller:
  
      $ trema run topology_controller
  
  then its trema-name is set to "topology_controller", 
  which is same as the executable name.  
  
  **Rule 3:** Trema-name can be overwritten using the `-n` or `--name`
  option given by the `trema run` command. 
  
      $ trema run "topology_controller -n topology"
      $ trema run "topology_controller --name=topology"

  Background: 
    Given a file named "print_pid.c" with:
      """
      #include <stdio.h>
      #include "trema.h"
      
      int
      main( int argc, char* argv[] ) {
        init_trema( &argc, &argv );
        
        if ( argc >= 2 ) {
          pid_t pid = get_pid_by_trema_name( argv[ 1 ] );
          if ( pid == -1 ) {
            printf( "No such process : %s\n", argv[ 1 ] );
          }
          else {
            printf( "PID of %s = %d\n", argv[ 1 ], pid );
          }
        }
        
        finalize_trema();
        return 0;
      }
      """
    And I compile "print_pid.c" into "print_pid"
    And a file named "c_controller.c" with:
      """
      #include "trema.h"
      
      int
      main( int argc, char* argv[] ) {
        init_trema( &argc, &argv );
        start_trema();
        return 0;
      }
      """
    And I compile "c_controller.c" into "c_controller"

  Scenario: Getting the PID of a Ruby controller process with a default name.
    Given a file named "RubyController.rb" with:
      """
      class RubyController < Controller
      end
      """
    And I run `trema run ./RubyController.rb -d`
    And wait until "RubyController" is up
    When I run `trema run "./print_pid RubyController"`
    Then the output should match:
      """
      PID of RubyController = \d+
      """

  Scenario: Getting the PID of a C controller process with a default name.
    Given I run `trema run "./c_controller" -d`
    And wait until "c_controller" is up
    When I run `trema run "./print_pid c_controller"`
    Then the output should match:
      """
      PID of c_controller = \d+
      """

  Scenario: Getting the PID of a C controller process renamed through CLI option '-n'.
    Given I run `trema run "./c_controller -n NewName" -d`
    And wait until "NewName" is up
    When I run `trema run "./print_pid NewName"`
    Then the output should match:
      """
      PID of NewName = \d+
      """

  Scenario: Getting the PID of a C controller process renamed through CLI option '--name'.
    Given I run `trema run "./c_controller --name=NewName" -d`
    And wait until "NewName" is up
    When I run `trema run "./print_pid NewName"`
    Then the output should match:
      """
      PID of NewName = \d+
      """

  @wip
  Scenario: Getting the PID of a Ruby controller process renamed through CLI option '-n'.
    Given a file named "RubyController.rb" with:
      """
      class RubyController < Controller
      end
      """
    And I run `trema run "./RubyController.rb -n NewName" -d`
    And wait until "NewName" is up
    When I run `trema run "./print_pid NewName"`
    Then the output should match:
      """
      PID of NewName = \d+
      """

  Scenario: Getting the PID of non-existent controller returns an error code -1.
    When I run `trema run "./print_pid NO_SUCH_TREMA_PROCESS"`
    Then the output should match:
      """
      No such process : NO_SUCH_TREMA_PROCESS
      """
