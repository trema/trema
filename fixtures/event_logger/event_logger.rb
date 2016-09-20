# frozen_string_literal: true
# logs Trema events
class EventLogger < Trema::Controller
  def start(args)
    logger.info "EventLogger#start (args = [#{args.join(', ')}])"
  end

  def switch_ready(dpid)
    logger.info "EventLogger#switch_ready (dpid = #{dpid.to_hex})"
  end
end
