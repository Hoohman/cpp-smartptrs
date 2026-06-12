#include "SharedPtr.h"
#include "WeakPtr.h"
#include <cassert>
#include <iostream>
#include <utility>

struct Node {
  int value;
  SharedPtr<Node> next;
  WeakPtr<Node> prev;
  bool &destroyed;

  Node(int v, bool &flag) : value(v), destroyed(flag) {}
  ~Node() { destroyed = true; }
};

// ---- SharedPtr ----

void test_shared_basic() {
  SharedPtr<int> p(new int(42));
  assert(p.use_count() == 1);
  assert(*p == 42);
  assert(p.get() != nullptr);
  assert(bool(p));
  std::cout << "test_shared_basic OK\n";
}

void test_shared_copy() {
  SharedPtr<int> p1(new int(10));
  SharedPtr<int> p2(p1);
  assert(p1.use_count() == 2);
  assert(p2.use_count() == 2);
  assert(*p2 == 10);
  std::cout << "test_shared_copy OK\n";
}

void test_shared_move() {
  SharedPtr<int> p1(new int(7));
  SharedPtr<int> p2(std::move(p1));
  assert(!p1);
  assert(p1.use_count() == 0);
  assert(*p2 == 7);
  assert(p2.use_count() == 1);
  std::cout << "test_shared_move OK\n";
}

void test_shared_reset() {
  SharedPtr<int> p(new int(1));
  p.reset(new int(2));
  assert(*p == 2);
  assert(p.use_count() == 1);

  p.reset();
  assert(!p);
  assert(p.use_count() == 0);
  std::cout << "test_shared_reset OK\n";
}

void test_shared_null() {
  SharedPtr<int> p;
  assert(!p);
  assert(p.get() == nullptr);
  assert(p.use_count() == 0);
  std::cout << "test_shared_null OK\n";
}

// ---- SharedPtr + WeakPtr ----

void test_weak_basic() {
  SharedPtr<int> sp(new int(99));
  WeakPtr<int> wp(sp);

  assert(!wp.expired());
  assert(wp.use_count() == 1);

  SharedPtr<int> locked = wp.lock();
  assert(locked);
  assert(*locked == 99);
  assert(sp.use_count() == 2);
  std::cout << "test_weak_basic OK\n";
}

void test_weak_expired() {
  WeakPtr<int> wp;
  {
    SharedPtr<int> sp(new int(5));
    wp = sp;
    assert(!wp.expired());
  }
  assert(wp.expired());
  assert(wp.use_count() == 0);

  SharedPtr<int> locked = wp.lock();
  assert(!locked);
  std::cout << "test_weak_expired OK\n";
}

void test_weak_copy() {
  SharedPtr<int> sp(new int(3));
  WeakPtr<int> w1(sp);
  WeakPtr<int> w2(w1);

  assert(!w1.expired());
  assert(!w2.expired());
  assert(sp.use_count() == 1);
  std::cout << "test_weak_copy OK\n";
}

// ---- Сycle ----

void test_cycle_broken_by_weak() {
  bool a_destroyed = false;
  bool b_destroyed = false;

  {
    SharedPtr<Node> a(new Node(1, a_destroyed));
    SharedPtr<Node> b(new Node(2, b_destroyed));

    a->next = b;
    b->prev = a;

    assert(a.use_count() == 1);
    assert(b.use_count() == 2);
  }

  assert(a_destroyed);
  assert(b_destroyed);
  std::cout << "test_cycle_broken_by_weak OK\n";
}

void test_self_assign() {
  SharedPtr<int> p(new int(42));
  p = p;
  assert(*p == 42);
  assert(p.use_count() == 1);
  std::cout << "test_self_assign OK\n";
}

int main() {
  test_shared_basic();
  test_shared_copy();
  test_shared_move();
  test_shared_reset();
  test_shared_null();

  test_weak_basic();
  test_weak_expired();
  test_weak_copy();

  test_cycle_broken_by_weak();

  test_self_assign();

  std::cout << "\nAll tests passed\n";
  return 0;
}