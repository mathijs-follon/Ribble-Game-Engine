#include <ribble/core/event.h>
#include "ribble/util/enum.h"

#include <algorithm>

using namespace ribble::core;
RIBBLE_ENUM_TO_STRING(EventBus::Failure,
                      case EventBus::Failure::EventBusNotInitialized : return "Event Bus Not Initialized";
                      case EventBus::Failure::InvalidEventType : return "Invalid Event Type";
                      case EventBus::Failure::ListenerAlreadyExists : return "Listener Already Exists";
                      case EventBus::Failure::ListenerNotFound : return "Listener Not Found";);


namespace ribble::core {
    EventListenerId EventListener::s_nextId = 1;

    EventListener::EventListener() : m_id(s_nextId++) {}

    EventListener::EventListener(EventHandler handler) : m_id(s_nextId++), m_handler(std::move(handler)) {}

    void EventListener::invoke(const std::shared_ptr<Event> &event) const {
        if (!m_enabled || !m_handler) {
            return;
        }

        if (event && !event->is_handled()) {
            m_handler(event);
        }
    }

    EventBus::EventBus() = default;

    EventListenerId EventBus::subscribe(std::type_index eventType, EventHandler handler, int priority) {
        const auto listener = std::make_shared<EventListener>(std::move(handler));
        listener->set_priority(priority);
        return subscribe(eventType, listener);
    }

    EventListenerId EventBus::subscribe(std::type_index eventType, std::shared_ptr<EventListener> listener) {
        if (!listener) {
            return 0;
        }

        EventListenerId id = listener->id();
        ListenerEntry entry;
        entry.listener = std::move(listener);
        entry.priority = entry.listener->priority();

        m_listeners[eventType].push_back(entry);
        sort_listeners(m_listeners[eventType]);

        return id;
    }

    Result<void, EventBus::Failure> EventBus::unsubscribe(std::type_index eventType, EventListenerId listenerId) {
        auto it = m_listeners.find(eventType);
        if (it == m_listeners.end()) {
            return Fail(RIBBLE_WARN(EventBus::Failure::ListenerNotFound, "when unsubscribing from event bus."));
        }

        auto &listeners = it->second;
        auto listenerIt = std::find_if(listeners.begin(), listeners.end(), [listenerId](const ListenerEntry &entry) {
            return entry.listener && entry.listener->id() == listenerId;
        });

        if (listenerIt == listeners.end()) {
            return Fail(RIBBLE_WARN(EventBus::Failure::ListenerNotFound, "when unsubscribing from event bus."));
        }

        listeners.erase(listenerIt);

        if (listeners.empty()) {
            m_listeners.erase(it);
        }

        return Ok();
    }

    void EventBus::dispatch(std::type_index eventType, const std::shared_ptr<Event> &event) {
        if (!m_enabled || !event) {
            return;
        }

        const QueuedEvent queuedEvent{.eventType = eventType, .event = event};

        m_eventQueue.push(queuedEvent);
    }

    void EventBus::dispatch_immediate(std::type_index eventType, const std::shared_ptr<Event> &event) {
        if (!m_enabled || !event) {
            return;
        }

        event->set_handled(false);

        const auto it = m_listeners.find(eventType);
        if (it == m_listeners.end()) {
            return;
        }

        invoke_listeners(it->second, event);
    }

    size_t EventBus::listener_count(std::type_index eventType) const {
        const auto it = m_listeners.find(eventType);
        if (it == m_listeners.end()) {
            return 0;
        }
        return it->second.size();
    }

    void EventBus::clear() { m_listeners.clear(); }

    void EventBus::clear(std::type_index eventType) { m_listeners.erase(eventType); }

    void EventBus::process_queue() {
        while (!m_eventQueue.empty()) {
            QueuedEvent queuedEvent = m_eventQueue.front();
            m_eventQueue.pop();
            dispatch_immediate(queuedEvent.eventType, queuedEvent.event);
        }
    }

    void EventBus::sort_listeners(std::vector<ListenerEntry> &listeners) {
        std::ranges::sort(listeners,
                          [](const ListenerEntry &a, const ListenerEntry &b) { return a.priority > b.priority; });
    }

    void EventBus::invoke_listeners(const std::vector<ListenerEntry> &listeners, const std::shared_ptr<Event> &event) {
        if (!event || event->is_handled()) {
            return;
        }

        for (const auto &entry: listeners) {
            if (!entry.listener) {
                continue;
            }

            entry.listener->invoke(event);

            // Stop propagation if event is handled or propagation is disabled
            if (event->is_handled() || !event->should_propagate()) {
                break;
            }
        }
    }
} // namespace ribble::core
