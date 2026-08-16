#pragma once
#include <vector>
#include <span>
#include "basis.hpp"

#ifndef NO_UNIQUE_ADDRESS_H
#define NO_UNIQUE_ADDRESS_H

#if defined(_MSC_VER)
    #define NO_UNIQUE_ADDRESS_ATTR [[msvc::no_unique_address]]
#elif defined(__GNUC__) || defined(__clang__)
    #define NO_UNIQUE_ADDRESS_ATTR [[no_unique_address]]
#else
    #define NO_UNIQUE_ADDRESS_ATTR
#endif

#endif // NO_UNIQUE_ADDRESS_H

namespace graph {

// -----------------------------------------------------------------------------
// 2) CSR (Compressed Sparse Row) - 行压缩
// -----------------------------------------------------------------------------
template <class EdgeW> struct CSR {
  std::vector<id_type> indptr;
  std::vector<id_type> indices;

  NO_UNIQUE_ADDRESS_ATTR
  std::conditional_t<Weighted<EdgeW>, std::vector<EdgeW>, WeightBox<void>> data;

  id_type num_rows;
  id_type num_cols;

  id_type rows() const { return num_rows; }
  id_type cols() const { return num_cols; }
  id_type nnz() const { return indices.size(); }

  std::span<const id_type> row_indices(id_type i) const {
    return {&indices[indptr[i]], &indices[indptr[i + 1]]};
  }

  auto row_weights(id_type i) const {
    if constexpr (Weighted<EdgeW>) {
      return std::span<const EdgeW>{&data[indptr[i]], &data[indptr[i + 1]]};
    } else {
      return std::span<const int>{};
    }
  }
};

// -----------------------------------------------------------------------------
// CSC (Compressed Sparse Column) - 列压缩
// -----------------------------------------------------------------------------
template <class EdgeW> struct CSC {
  std::vector<id_type> indptr;
  std::vector<id_type> indices;

  NO_UNIQUE_ADDRESS_ATTR
  std::conditional_t<Weighted<EdgeW>, std::vector<EdgeW>, WeightBox<void>> data;

  id_type num_rows;
  id_type num_cols;

  id_type rows() const { return num_rows; }
  id_type cols() const { return num_cols; }
  id_type nnz() const { return indices.size(); }

  std::span<const id_type> col_indices(id_type j) const {
    return {&indices[indptr[j]], &indices[indptr[j + 1]]};
  }
};

} // namespace graph
