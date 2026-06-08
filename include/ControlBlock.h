#pragma once

#include <cstddef>

template <typename T> struct ControlBlock {
  T *ptr_;
  size_t shared_count_{0};
  size_t weak_count_{0};
};