#pragma once
#include <functional>
#include <memory>
#include <queue>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "fail.h"

namespace ribble::core {

    class Node;
    class Component;

    class Event {
    public:
        Event() = default;
        virtual ~Event() = default;

        Event(const Event &) = default;
        Event &operator=(const Event &) = default;
        Event(Event &&) noexcept = default;
        Event &operator=(Event &&) noexcept = default;

        [[nodiscard]] virtual size_t type_id() const = 0;
        [[nodiscard]] virtual std::string type_str() const = 0;

        void set_handled(bool handled) { m_handled = handled; }
        [[nodiscard]] bool is_handled() const { return m_handled; }

        void set_propagate(bool propagate) { m_propagate = propagate; }
        [[nodiscard]] bool should_propagate() const { return m_propagate; }

    private:
        bool m_handled{false};
        bool m_propagate{true};
    };


    // Todo: move to correct file
    class ComponentEvent : public Event {
    public:
        void set_source_component(std::shared_ptr<Component> component) { m_sourceComponent = std::move(component); }
        [[nodiscard]] std::shared_ptr<Component> source_component() const { return m_sourceComponent; }

        void set_source_node(std::shared_ptr<Node> node) { m_sourceNode = std::move(node); }
        [[nodiscard]] std::shared_ptr<Node> source_node() const { return m_sourceNode; }

    private:
        std::shared_ptr<Component> m_sourceComponent;
        std::shared_ptr<Node> m_sourceNode;
    };

    using EventHandler = std::function<void(const std::shared_ptr<Event> &)>;
    using EventListenerId = size_t;

    class EventListener {
    public:
        EventListener();
        explicit EventListener(EventHandler handler);
        ~EventListener() = default;

        EventListener(const EventListener &) = default;
        EventListener &operator=(const EventListener &) = default;

        EventListener(EventListener &&) noexcept = default;
        EventListener &operator=(EventListener &&) noexcept = default;

        void set_handler(EventHandler handler) { m_handler = std::move(handler); }
        [[nodiscard]] const EventHandler &handler() const { return m_handler; }

        void set_priority(int priority) { m_priority = priority; }
        [[nodiscard]] int priority() const { return m_priority; }

        void set_enabled(bool enabled) { m_enabled = enabled; }
        [[nodiscard]] bool is_enabled() const { return m_enabled; }

        [[nodiscard]] EventListenerId id() const { return m_id; }

        void invoke(const std::shared_ptr<Event> &event) const;

    private:
        static EventListenerId s_nextId;
        EventListenerId m_id;
        EventHandler m_handler;
        int m_priority{0};
        bool m_enabled{true};
    };

    class EventBus {
    public:
        enum class Failure { ListenerNotFound, ListenerAlreadyExists, InvalidEventType, EventBusNotInitialized };

        EventBus();
        ~EventBus() = default;

        EventBus(const EventBus &) = delete;
        EventBus &operator=(const EventBus &) = delete;

        EventBus(EventBus &&) noexcept = default;
        EventBus &operator=(EventBus &&) noexcept = default;

        template<typename T>
            requires std::derived_from<T, Event>
        EventListenerId subscribe(EventHandler handler, int priority = 0);

        template<typename T>
            requires std::derived_from<T, Event>
        EventListenerId subscribe(std::shared_ptr<EventListener> listener);

        EventListenerId subscribe(std::type_index eventType, EventHandler handler, int priority = 0);
        EventListenerId subscribe(std::type_index eventType, std::shared_ptr<EventListener> listener);

        template<typename T>
            requires std::derived_from<T, Event>
        Result<void, Failure> unsubscribe(EventListenerId listenerId);
        Result<void, Failure> unsubscribe(std::type_index eventType, EventListenerId listenerId);

        template<typename T>
            requires std::derived_from<T, Event>
        void dispatch(const std::shared_ptr<T> &event);

        void dispatch(std::type_index eventType, const std::shared_ptr<Event> &event);

        template<typename T>
            requires std::derived_from<T, Event>
        void dispatch_immediate(const std::shared_ptr<T> &event);

        void dispatch_immediate(std::type_index eventType, const std::shared_ptr<Event> &event);

        template<typename T>
            requires std::derived_from<T, Event>
        [[nodiscard]] size_t listener_count() const;

        [[nodiscard]] size_t listener_count(std::type_index eventType) const;

        void clear();
        void clear(std::type_index eventType);

        void process_queue();

        void set_enabled(bool enabled) { m_enabled = enabled; }
        [[nodiscard]] bool is_enabled() const { return m_enabled; }

    private:
        struct ListenerEntry {
            std::shared_ptr<EventListener> listener;
            int priority;
        };

        struct QueuedEvent {
            std::type_index eventType;
            std::shared_ptr<Event> event;
        };

        static void sort_listeners(std::vector<ListenerEntry> &listeners);
        static void invoke_listeners(const std::vector<ListenerEntry> &listeners, const std::shared_ptr<Event> &event);

        std::unordered_map<std::type_index, std::vector<ListenerEntry>> m_listeners;
        std::queue<QueuedEvent> m_eventQueue;
        bool m_enabled{true};
    };

    template<typename T>
        requires std::derived_from<T, Event>
    EventListenerId EventBus::subscribe(EventHandler handler, int priority) {
        const auto eventType = std::type_index(typeid(T));
        const auto listener = std::make_shared<EventListener>(handler);
        listener->set_priority(priority);
        return subscribe(eventType, listener);
    }

    template<typename T>
        requires std::derived_from<T, Event>
    EventListenerId EventBus::subscribe(std::shared_ptr<EventListener> listener) {
        const auto eventType = std::type_index(typeid(T));
        return subscribe(eventType, std::move(listener));
    }

    template<typename T>
        requires std::derived_from<T, Event>
    Result<void, EventBus::Failure> EventBus::unsubscribe(EventListenerId listenerId) {
        auto eventType = std::type_index(typeid(T));
        return unsubscribe(eventType, listenerId);
    }

    template<typename T>
        requires std::derived_from<T, Event>
    void EventBus::dispatch(const std::shared_ptr<T> &event) {
        auto eventType = std::type_index(typeid(T));
        dispatch(eventType, event);
    }

    template<typename T>
        requires std::derived_from<T, Event>
    void EventBus::dispatch_immediate(const std::shared_ptr<T> &event) {
        auto eventType = std::type_index(typeid(T));
        dispatch_immediate(eventType, event);
    }

    template<typename T>
        requires std::derived_from<T, Event>
    size_t EventBus::listener_count() const {
        const auto eventType = std::type_index(typeid(T));
        return listener_count(eventType);
    }

} // namespace ribble::core
