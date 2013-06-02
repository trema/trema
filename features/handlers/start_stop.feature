Feature: start/stop handler

  @wip
  Scenario: start handler
    Given a file named "start-handler.rb" with:
    """
    class StartHandler < Controller
      def start
        info "Controller is started"
      end
    end
    """
    When I run `trema run ./start-handler.rb` interactively
    Then the output should contain "Controller is started" within the timeout period

  @wip
  Scenario: stop handler
    Given a file named "stop-handler.rb" with:
    """
    class StopHandler < Controller
      def stop
        info "Controller is terminated"
      end
    end
    """
     And  I run `trema run ./stop-handler.rb` interactively
     And wait until "StopHandler" is up
    When I run `trema killall`
    Then the output should contain "Controller is terminated" within the timeout period
