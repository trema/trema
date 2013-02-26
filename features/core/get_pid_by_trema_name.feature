Feature: get_pid_by_trema_name()
  
  The function get_pid_by_trema_name() returns the pid of the process 
  specfied as the `name` argument. 
  If the `name` was not found inside Trema's name space, the function returns -1.
  
      pid_t get_pid_by_trema_name( const char *name )
  
  Here the `name` argument is a controller process's name called *trema-name*.  
  Inside the Trema's name space, Trema uses the
  trema-name as a unique identifier to distinguish each controller processes. 
  It is determined automatically by the following rules:
   
  **Rule 1:** When you start a Ruby controller: 
  
      \# topology-controller.rb
      class TopologyController < Controller
        \# ...
      end
  
      $ trema run topology-controller.rb
  
  then its trema-name is set to "TopologyController", 
  which is taken from its class name.  
  
  **Rule 2:** When you start a C controller:
  
      $ trema run topology_controller
  
  then its trema-name is set to 'topology_controller', 
  which is taken from its executable name.  
  
  **Rule 3:** Trema-name can be overwritten using the `-n` or `--name`
  option given through `trema run`:
  
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
          printf( "PID of %s = %d\n", argv[ 1 ], pid );
        }
        start_trema_up();
        start_trema_down();
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

  Scenario: Getting the pid of a Ruby controller process with default name.
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

  Scenario: Getting the pid of a C controller process with default name.
    Given I run `trema run "./c_controller" -d`
    And wait until "c_controller" is up
    When I run `trema run "./print_pid c_controller"`
    Then the output should match:
      """
      PID of c_controller = \d+
      """

  Scenario: Getting the pid of a C controller process renamed through CLI option '-n'.
    Given I run `trema run "./c_controller -n NewName" -d`
    And wait until "NewName" is up
    When I run `trema run "./print_pid NewName"`
    Then the output should match:
      """
      PID of NewName = \d+
      """

  Scenario: Getting the pid of a C controller process renamed through CLI option '--name'.
    Given I run `trema run "./c_controller --name=NewName" -d`
    And wait until "NewName" is up
    When I run `trema run "./print_pid NewName"`
    Then the output should match:
      """
      PID of NewName = \d+
      """

  @wip
  Scenario: Getting the pid of a Ruby controller process renamed through CLI option '-n'.
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

  Scenario: Getting the pid of non-existent controller returns an error code -1.
    When I run `trema run "./print_pid NO_SUCH_TREMA_PROCESS"`
    Then the output should match:
      """
      PID of NO_SUCH_TREMA_PROCESS = -1
      """
