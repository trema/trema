Feature: get_pid_by_trema_name
  
      pid_t get_pid_by_trema_name( const char *name )
  
  Trema C API get_pid_by_trema_name() returns the pid of the trema process 
  or it returns -1 if the specified trema process did not exist. 
  
  The `name` argument is a trema name which is a name used inside trema's 
  name space and it may not always match the process name in the Operating System.  
  
  For example when the C controller is started like: 
  
      $ trema run topology_controller
  
  then the trema name will be the same as the executable name, 'topology_controller'.  
  
  Trema name can be explicitly specified using the -n or --name option:
  
      $ trema run topology_controller -n logical_topology
  
      $ trema run topology_controller --name=transport_topology
   
   each controller instance will have the trema name 'logical_topology' 
   and 'transport_topology' respectively.  
  
  If the trema process is a Ruby controller, 
  the trema name is equal to the class name by default. 
  
      class MyController < Controller
        \# ...
      end
  
  In the above example the trema name will be 'MyController'.  
  
  Trema name can also be explicitly given by overriding `Controller#name` method.
  
      class PrefixToAvoidConflict_MyController < Controller
        def name
          "MyController"
        end
      end
  
  In the above example the trema name will be 'MyController'.  
  

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

  Scenario: returns the pid for Ruby Controller with explicitly given name.
    Given a file named "RenamedRubyController.rb" with:
      """
      class RenamedRubyController < Controller
        def name
          "FriendlyName"
        end
      end
      """
    And I run `trema run "./RenamedRubyController.rb" -d`
    And wait until "FriendlyName" is up
    When I run `trema run "./get_pid_for_trema_process FriendlyName"` interactively
    Then the output should match:
      """
      PID of FriendlyName = \d+
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

  Scenario: returns -1 when specfied controller name does not exists.
    Given I compile "empty_c_controller.c" into "empty_c_controller"
    And I run `trema run "./empty_c_controller" -d`
    And wait until "empty_c_controller" is up
    When I run `trema run "./get_pid_for_trema_process PhantomController"` interactively
    Then the output should match:
      """
      PID of PhantomController = -1
      """
