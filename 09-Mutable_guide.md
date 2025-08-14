
# 🔧 `mutable` in C++

A detailed guide to the **`mutable` keyword** in C++:  
What it is, why it matters, and how to use it effectively.

---

## 📌 What is `mutable`?

In C++, the `mutable` keyword **allows a non-static class member variable to be modified inside a `const` member function**.

```cpp
class Example {
public:
  int GetCount() const {
    ++m_count; // allowed because m_count is mutable
    return m_count;
  }

private:
  mutable int m_count = 0;
};
```

### Key Points
- Works only for **non-static** member variables.
- Overrides the immutability implied by `const` member functions.
- Useful for **logical constness** — modifying internal state without changing the logical behavior of the object.

---

## 🧠 Why Use `mutable`?

| **Use Case**     | **Description** |
|------------------|-----------------|
| **Caching**      | Store computed results without breaking `const`. |
| **Dirty Flags**  | Track internal state changes for lazy updates. |
| **Debug Counters** | Count accesses or log events inside `const` methods. |
| **Logical Constness** | Preserve external immutability while allowing internal optimization. |

---

## 🔍 Example: Bounding Box Cache

```cpp
class BoundingBox { /* ... */ };

class Mesh {
public:
  const BoundingBox& GetBoundingBox() const {
    if (m_dirty) {
      m_bbox = ComputeBoundingBox();
      m_dirty = false;
    }
    return m_bbox;
  }

private:
  BoundingBox ComputeBoundingBox() const;

  mutable BoundingBox m_bbox;
  mutable bool m_dirty = true;
};
```

### Explanation
- `GetBoundingBox()` is `const`.
- `m_bbox` and `m_dirty` are updated internally, thanks to `mutable`.
- This is a **classic use case**: **lazy evaluation** with a **dirty flag pattern**.

---

## 🛠 More Practical Examples

### 1. Debugging Counters
```cpp
class DebugLogger {
public:
  void Log() const {
    ++m_callCount; // Track number of calls
    std::cout << "Logging..." << std::endl;
  }

  int GetCallCount() const { return m_callCount; }

private:
  mutable int m_callCount = 0;
};
```

### 2. Memoization
```cpp
#include <unordered_map>

class Fibonacci {
public:
  int GetValue(int n) const {
    if (cache.find(n) == cache.end()) {
      cache[n] = (n <= 1) ? n : GetValue(n - 1) + GetValue(n - 2);
    }
    return cache[n];
  }

private:
  mutable std::unordered_map<int, int> cache;
};
```

---

## ⚠️ Things to Watch Out For

- **Non-static only**: `mutable` cannot be applied to static members.
- **Thread safety**: `mutable` does not make code thread-safe. Use locks or atomics when needed.
- **Overuse risk**: Too much `mutable` can obscure an object’s state, breaking `const` expectations.

---

## ✅ When to Use

Use `mutable` when:
- You need to **cache** or track **internal state** in a `const` method.
- You want to preserve **logical constness**.
- You’re implementing **performance optimizations** like lazy updates.

---

## ❌ When Not to Use

Avoid `mutable` when:
- It **breaks the expected behavior** of `const` objects.
- It introduces **hidden side effects**.
- You are in a **multithreaded context** without proper synchronization.

---

## 📚 Related Concepts
- `const_cast`
- Const member functions
- Logical vs. physical constness
- Lazy evaluation
- Dirty flag pattern

---

> **Summary:**  
> *"`mutable` is a bridge between const-correctness and practical optimization."*
