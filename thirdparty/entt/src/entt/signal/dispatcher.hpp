#ifndef ENTT_SIGNAL_DISPATCHER_HPP
#define ENTT_SIGNAL_DISPATCHER_HPP


#include "EASTL/vector.h"
#include "EASTL/memory.h"
#include "EASTL/utility.h"
#include "EASTL/algorithm.h"
#include "EASTL/type_traits.h"
#include "EASTL/unique_ptr.h"
#include <cstddef>
#include "../config/config.h"
#include "../core/fwd.hpp"
#include "../core/type_info.hpp"
#include "sigh.hpp"


namespace entt {


/**
 * @brief Basic dispatcher implementation.
 *
 * A dispatcher can be used either to trigger an immediate event or to enqueue
 * events to be published all together once per tick.<br/>
 * Listeners are provided in the form of member functions. For each event of
 * type `Event`, listeners are such that they can be invoked with an argument of
 * type `const Event &`, no matter what the return type is.
 *
 * The dispatcher creates instances of the `sigh` class internally. Refer to the
 * documentation of the latter for more details.
 */
class dispatcher {
    struct basic_pool {
        virtual ~basic_pool() = default;
        virtual void publish() = 0;
        virtual void clear() ENTT_NOEXCEPT = 0;
        virtual id_type type_id() const ENTT_NOEXCEPT = 0;
    };

    template<typename Event>
    struct pool_handler final: basic_pool {
        using signal_type = sigh<void(const Event &)>;
        using sink_type = typename signal_type::sink_type;

        void publish() override {
            const auto length = events.size();

            for(std::size_t pos{}; pos < length; ++pos) {
                signal.publish(events[pos]);
            }

            events.erase(events.cbegin(), events.cbegin()+length);
        }

        void clear() ENTT_NOEXCEPT override {
            events.clear();
        }

        sink_type sink() ENTT_NOEXCEPT {
            return entt::sink{signal};
        }

        template<typename... Args>
        void trigger(Args &&... args) {
            signal.publish(Event{eastl::forward<Args>(args)...});
        }

        template<typename... Args>
        void enqueue(Args &&... args) {
            events.emplace_back(eastl::forward<Args>(args)...);
        }

        id_type type_id() const ENTT_NOEXCEPT override {
            return type_info<Event>::id();
        }

    private:
        signal_type signal{};
        eastl::vector<Event> events;
    };

    template<typename Event>
    pool_handler<Event> & assure() {
        static_assert(eastl::is_same_v<Event, eastl::decay_t<Event>>);

        if constexpr(has_type_index_v<Event>) {
            const auto index = type_index<Event>::value();

            if(!(index < pools.size())) {
                pools.resize(index+1);
            }

            if(!pools[index]) {
                pools[index].reset(new pool_handler<Event>{});
            }

            return static_cast<pool_handler<Event> &>(*pools[index]);
        } else {
            auto it = eastl::find_if(pools.begin(), pools.end(), [id = type_info<Event>::id()](const auto &cpool) { return id == cpool->type_id(); });
            return static_cast<pool_handler<Event> &>(it == pools.cend() ? *pools.emplace_back(new pool_handler<Event>{}) : **it);
        }
    }

public:
    /**
     * @brief Returns a sink object for the given event.
     *
     * A sink is an opaque object used to connect listeners to events.
     *
     * The function type for a listener is:
     * @code{.cpp}
     * void(const Event &);
     * @endcode
     *
     * The order of invocation of the listeners isn't guaranteed.
     *
     * @sa sink
     *
     * @tparam Event Type of event of which to get the sink.
     * @return A temporary sink object.
     */
    template<typename Event>
    auto sink() {
        return assure<Event>().sink();
    }

    /**
     * @brief Triggers an immediate event of the given type.
     *
     * All the listeners registered for the given type are immediately notified.
     * The event is discarded after the execution.
     *
     * @tparam Event Type of event to trigger.
     * @tparam Args Types of arguments to use to construct the event.
     * @param args Arguments to use to construct the event.
     */
    template<typename Event, typename... Args>
    void trigger(Args &&... args) {
        assure<Event>().trigger(eastl::forward<Args>(args)...);
    }

    /**
     * @brief Triggers an immediate event of the given type.
     *
     * All the listeners registered for the given type are immediately notified.
     * The event is discarded after the execution.
     *
     * @tparam Event Type of event to trigger.
     * @param event An instance of the given type of event.
     */
    template<typename Event>
    void trigger(Event &&event) {
        assure<eastl::decay_t<Event>>().trigger(eastl::forward<Event>(event));
    }

    /**
     * @brief Enqueues an event of the given type.
     *
     * An event of the given type is queued. No listener is invoked. Use the
     * `update` member function to notify listeners when ready.
     *
     * @tparam Event Type of event to enqueue.
     * @tparam Args Types of arguments to use to construct the event.
     * @param args Arguments to use to construct the event.
     */
    template<typename Event, typename... Args>
    void enqueue(Args &&... args) {
        assure<Event>().enqueue(eastl::forward<Args>(args)...);
    }

    /**
     * @brief Enqueues an event of the given type.
     *
     * An event of the given type is queued. No listener is invoked. Use the
     * `update` member function to notify listeners when ready.
     *
     * @tparam Event Type of event to enqueue.
     * @param event An instance of the given type of event.
     */
    template<typename Event>
    void enqueue(Event &&event) {
        assure<eastl::decay_t<Event>>().enqueue(eastl::forward<Event>(event));
    }

    /**
     * @brief Discards all the events queued so far.
     *
     * If no types are provided, the dispatcher will clear all the existing
     * pools.
     *
     * @tparam Event Type of events to discard.
     */
    template<typename... Event>
    void clear() {
        if constexpr(sizeof...(Event) == 0) {
            for(auto &&cpool: pools) {
                if(cpool) {
                    cpool->clear();
                }
            }
        } else {
            (assure<Event>().clear(), ...);
        }
    }

    /**
     * @brief Delivers all the pending events of the given type.
     *
     * This method is blocking and it doesn't return until all the events are
     * delivered to the registered listeners. It's responsibility of the users
     * to reduce at a minimum the time spent in the bodies of the listeners.
     *
     * @tparam Event Type of events to send.
     */
    template<typename Event>
    void update() {
        assure<Event>().publish();
    }

    /**
     * @brief Delivers all the pending events.
     *
     * This method is blocking and it doesn't return until all the events are
     * delivered to the registered listeners. It's responsibility of the users
     * to reduce at a minimum the time spent in the bodies of the listeners.
     */
    void update() const {
        for(auto pos = pools.size(); pos; --pos) {
            if(auto &&cpool = pools[pos-1]; cpool) {
                cpool->publish();
            }
        }
    }

private:
    eastl::vector<eastl::unique_ptr<basic_pool>> pools;
};


}


#endif
