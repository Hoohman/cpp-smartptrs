#pragma once

#include "ControlBlock.h"
#include <cstddef>
#include <utility>

template <typename T> class SharedPtr;

template <typename T> class WeakPtr {
public:
  WeakPtr();
  WeakPtr(const SharedPtr<T> &shared);
  WeakPtr(const WeakPtr &other);
  WeakPtr(WeakPtr &&other) noexcept;
  ~WeakPtr();

  WeakPtr &operator=(const SharedPtr<T> &shared);
  WeakPtr &operator=(const WeakPtr &other);
  WeakPtr &operator=(WeakPtr &&other) noexcept;

  SharedPtr<T> lock() const;
  bool expired() const;
  size_t use_count() const;

private:
  void release();

  ControlBlock<T> *cb_;
};

template <typename T> WeakPtr<T>::WeakPtr() : cb_(nullptr) {}

template <typename T> WeakPtr<T>::WeakPtr(const SharedPtr<T> &shared) {
  if (shared.cb_ != nullptr) {
    cb_ = shared.cb_;
    cb_->weak_count_++;
  } else {
    cb_ = nullptr;
  }
}

template <typename T> WeakPtr<T>::WeakPtr(const WeakPtr &other) {
  if (other.cb_ != nullptr) {
    cb_ = other.cb_;
    cb_->weak_count_++;
  } else {
    cb_ = nullptr;
  }
}

template <typename T> WeakPtr<T>::WeakPtr(WeakPtr &&other) noexcept {
  cb_ = other.cb_;
  other.cb_ = nullptr;
}

template <typename T> WeakPtr<T>::~WeakPtr() { release(); }

template <typename T>
WeakPtr<T> &WeakPtr<T>::operator=(const SharedPtr<T> &shared) {
  WeakPtr<T> temp(shared);
  std::swap(cb_, temp.cb_);
  return *this;
}

template <typename T> WeakPtr<T> &WeakPtr<T>::operator=(const WeakPtr &other) {
  WeakPtr<T> temp(other);
  std::swap(cb_, temp.cb_);
  return *this;
}

template <typename T>
WeakPtr<T> &WeakPtr<T>::operator=(WeakPtr &&other) noexcept {
  std::swap(cb_, other.cb_);
  return *this;
}

template <typename T> SharedPtr<T> WeakPtr<T>::lock() const {
  SharedPtr<T> temp;
  if (cb_ != nullptr && cb_->shared_count_ > 0) {
    temp.cb_ = cb_;
    cb_->shared_count_++;
    return temp;
  }

  return temp;
}

template <typename T> bool WeakPtr<T>::expired() const {
  return use_count() == 0;
}

template <typename T> size_t WeakPtr<T>::use_count() const {
  if (cb_ != nullptr) {
    return cb_->shared_count_;
  }

  return 0;
}

template <typename T> void WeakPtr<T>::release() {
  if (cb_ == nullptr) {
    return;
  }
  cb_->weak_count_--;

  if (cb_->weak_count_ == 0 && cb_->shared_count_ == 0) {
    delete cb_;
  }

  cb_ = nullptr;
}