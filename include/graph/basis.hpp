#pragma once
#include <cstdint>
#include <type_traits>
#include <utility>

namespace graph {

using id_type = uint32_t;

// ---------- 1) 权重 mixin：void => 真·零开销 且 API 分离 ----------
template <class W>
concept Weighted = !std::is_void_v<W>;

template <class W> struct WeightBox {
  W value{};
};

template <> struct WeightBox<void> {};

template <class W> struct WeightMixin : private WeightBox<W> {
  using WB = WeightBox<W>;
  WeightMixin() = default;

  template <class U = W>
  explicit WeightMixin(const U &w)
    requires Weighted<U>
  {
    this->WB::value = w;
  }

  template <class U = W>
  explicit WeightMixin(U &&w)
    requires Weighted<U>
  {
    this->WB::value = std::move(w);
  }

  template <class U = W>
  U &weight()
    requires Weighted<U>
  {
    return this->WB::value;
  }

  template <class U = W>
  const U &weight() const
    requires Weighted<U>
  {
    return this->WB::value;
  }
};

} // namespace graph
