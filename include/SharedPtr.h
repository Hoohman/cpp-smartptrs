#pragma once

#include "ControlBlock.h"
#include <cassert>
#include <cstddef>
#include <utility>

template <typename T> class WeakPtr;

template <typename T> class SharedPtr {
public:
  explicit SharedPtr(T *ptr = nullptr);
  SharedPtr(const SharedPtr &other);
  SharedPtr(SharedPtr &&other) noexcept;
  ~SharedPtr();

  SharedPtr &operator=(const SharedPtr &other);
  SharedPtr &operator=(SharedPtr &&other) noexcept;

  T &operator*() const;
  T *operator->() const;
  explicit operator bool() const;

  void reset(T *ptr = nullptr);
  T *get() const;
  size_t use_count() const;

  friend class WeakPtr<T>;

private:
  void release();

  ControlBlock<T> *cb_{nullptr};
};

template <typename T> SharedPtr<T>::SharedPtr(T *ptr) {
  if (ptr != nullptr) {
    try {
      cb_ = new ControlBlock<T>;
      cb_->ptr_ = ptr;
      cb_->shared_count_ = 1;
    } catch (...) {
      delete ptr;
      throw;
    }
  } else {
    cb_ = nullptr;
  }
}

template <typename T> SharedPtr<T>::SharedPtr(const SharedPtr &other) {
  if (other.cb_ != nullptr) {
    cb_ = other.cb_;
    cb_->shared_count_++;
  } else {
    cb_ = nullptr;
  }
}

template <typename T> SharedPtr<T>::SharedPtr(SharedPtr &&other) noexcept {
  cb_ = other.cb_;
  other.cb_ = nullptr;
}

template <typename T> SharedPtr<T>::~SharedPtr() { release(); }

template <typename T>
SharedPtr<T> &SharedPtr<T>::operator=(const SharedPtr &other) {
  SharedPtr<T> temp(other);
  std::swap(cb_, temp.cb_);
  return *this;
}

template <typename T>
SharedPtr<T> &SharedPtr<T>::operator=(SharedPtr &&other) noexcept {
  if (this != &other) {
    release();
    cb_ = other.cb_;
    other.cb_ = nullptr;
  }
  return *this;
}

template <typename T> T &SharedPtr<T>::operator*() const {
  assert(cb_ != nullptr && cb_->ptr_ != nullptr);
  return *(cb_->ptr_);
}

template <typename T> T *SharedPtr<T>::operator->() const {
  if (cb_ != nullptr) {
    return cb_->ptr_;
  }
  return nullptr;
}

template <typename T> SharedPtr<T>::operator bool() const {
  return cb_ != nullptr && cb_->ptr_ != nullptr;
}

template <typename T> void SharedPtr<T>::reset(T *ptr) {
  SharedPtr<T> temp(ptr);
  std::swap(cb_, temp.cb_);
}

template <typename T> T *SharedPtr<T>::get() const {
  return cb_ ? cb_->ptr_ : nullptr;
}

template <typename T> size_t SharedPtr<T>::use_count() const {
  if (cb_ != nullptr) {
    return cb_->shared_count_;
  }

  return 0;
}

template <typename T> void SharedPtr<T>::release() {
  if (cb_ == nullptr) {
    return;
  }
  cb_->shared_count_--;
  if (cb_->shared_count_ == 0) {
    bool no_weak_refs = (cb_->weak_count_ == 0);
    delete cb_->ptr_;
    if (no_weak_refs) {
      delete cb_;
    }
  }
  cb_ = nullptr;
}