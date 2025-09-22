#pragma once

#include <functional>
#include <unordered_map>
#include <vector>

using SubscriberId = int;

template <typename Return, typename... Arguments>
class Publisher
{
  private:
    using Handler = std::function<Return(Arguments...)>;
    std::unordered_map<SubscriberId, Handler> subscribers;
    SubscriberId nextAvailableId = 0;

  public:
    SubscriberId subscribe(const Handler& handler);
    bool unsubscribe(const SubscriberId id);
    auto notify(Arguments... args);
};

template <typename Return, typename... Arguments>
inline SubscriberId Publisher<Return, Arguments...>::subscribe(const Handler& handler)
{
    const SubscriberId id = nextAvailableId++;
    subscribers[id] = handler;
    return id;
}

template <typename Return, typename... Arguments>
inline bool Publisher<Return, Arguments...>::unsubscribe(const SubscriberId id)
{
    return subscribers.erase(id) > 0;
}

template <typename Return, typename... Arguments>
inline auto Publisher<Return, Arguments...>::notify(Arguments... args)
{
    // Handle no returns.
    if constexpr (std::is_void_v<Return>)
    {
        for (const auto& [id, handler] : subscribers)
        {
            handler(args...);
        }
        return;
    }
    else
    {
        std::vector<Return> results;
        results.reserve(subscribers.size());
        for (const auto& [id, handler] : subscribers)
        {
            results.push_back(handler(args...));
        }
        return results;
    }
}
