#ifndef GMCS_POINTER_STACK_H_
#define GMCS_POINTER_STACK_H_

template <typename T>
struct pointer_stack {

  T count;
  T * p;

  pointer_stack()
      : count{0},
        p{nullptr} {
  }

  T & size() {
    return count;
  }

  T const & size() const {
    return count;
  }

  void push(T const & v) {
    p[count] = v;
    ++count;
  }

  void pop() {
    --count;
  }

  friend bool operator==(pointer_stack<T> const & lhs, pointer_stack<T> const & rhs) {
    return lhs.size() == rhs.size() && memcmp(lhs.p, rhs.p, lhs.size()) == 0;
  }
};

#endif  // GMCS_POINTER_STACK_H_
