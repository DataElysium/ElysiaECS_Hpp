#pragma once

#include <cstdint>
#include <functional>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "elysia/entity.hpp"
#include "elysia/meta.hpp"

namespace elysia {

enum class ObserverEvent : uint8_t {
    OnAdd, OnRemove
};

struct ObserverKey {
    ObserverEvent event;
    uint64_t component_id;

    bool operator==(const ObserverKey& other) const {
        return event == other.event && component_id == other.component_id;
    }
};

} // namespace elysia

namespace std {
template<> struct hash<elysia::ObserverKey> {
    size_t operator()(const elysia::ObserverKey& k) const {
        return (static_cast<size_t>(k.component_id) << 1) | static_cast<size_t>(k.event);
    }
};
} // namespace std

namespace elysia {

using ObserverCallback = std::function<void(Entity)>;

class ObserverRegistry {
public:
    void on_add(uint64_t comp_id, ObserverCallback cb) {
        observers_[{ObserverEvent::OnAdd, comp_id}].push_back(std::move(cb));
    }

    void on_remove(uint64_t comp_id, ObserverCallback cb) {
        observers_[{ObserverEvent::OnRemove, comp_id}].push_back(std::move(cb));
    }

    void notify(ObserverEvent event, uint64_t comp_id, Entity e) {
        auto it = observers_.find({event, comp_id});
        if (it == observers_.end()) return;
        for (const auto& cb : it->second) cb(e);
    }

    template <typename T>
    void on_add(ObserverCallback cb) { on_add(TypeTraits<std::remove_cvref_t<T>>::id, std::move(cb)); }

    template <typename T>
    void on_remove(ObserverCallback cb) { on_remove(TypeTraits<std::remove_cvref_t<T>>::id, std::move(cb)); }

    [[nodiscard]] bool has_observer(ObserverEvent event, uint64_t comp_id) const {
        return observers_.contains({event, comp_id});
    }

private:
    std::unordered_map<ObserverKey, std::vector<ObserverCallback>> observers_;
};

} // namespace elysia

