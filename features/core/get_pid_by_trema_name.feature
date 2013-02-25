Feature: get_pid_by_trema_name()
  
  The function get_pid_by_trema_name() returns the pid of the process 
  specfied as the `name` argument.  
  
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
  
  **Rule 3:** Trema name can be overwritten using the `-n` or `--name`
  option given through `trema run`:
  
      $ trema run "topology_controller -n topology"
      $ trema run "topology_controller --name=topology"
  
  
  If the `name` was not found inside Trema's name space, the function returns -1.

  Background: 
    Given a file named "get_pid_for_trema_process.c" with:
      """
      #include <stdio.h>
      #include "trema.h"
      
      int
      main( int argc, char* argv[] ) {
        init_trema( &argc, &argv );
        
        int i;
        for ( i = 1 ; i < argc ; ++i ) {
          pid_t pid = get_pid_by_trema_name( argv[ i ] );
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
    And I run `trema run "./empty_c_controller -n InstanceName" -d`
    And wait until "InstanceName" is up
    When I run `trema run "./get_pid_for_trema_process InstanceName"` interactively
    Then the output should match:
      """
      PID of InstanceName = \d+
      """

  Scenario: return the pid of C Controller renamed through CLI option '--name'
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller --name=ShortName" -d`
    And wait until "ShortName" is up
    When I run `trema run "./get_pid_for_trema_process ShortName"` interactively
    Then the output should match:
      """
      PID of ShortName = \d+
      """

  @wip
  Scenario: returns the pid for Ruby Controller renamed through CLI option '-n'
    Given a file named "EmptyRubyController.rb" with:
      """
      class EmptyRubyController < Controller
      end
      """
    And I run `trema run "./EmptyRubyController.rb -n InstanceName" -d`
    And wait until "InstanceName" is up
    When I run `trema run "./get_pid_for_trema_process InstanceName"` interactively
    Then the output should match:
      """
      PID of InstanceName = \d+
      """

  Scenario: returns -1 when specfied controller name does not exists.
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller" -d`
    And wait until "empty_c_controller" is up
    When I run `trema run "./get_pid_for_trema_process PhantomController"` interactively
    Then the output should match:
      """
      PID of PhantomController = -1
      """
