# C++ Exception Handling

## 📌 What is an Exception?
An **exception** represents an unexpected event that occurs during program execution, which disrupts the normal flow of instructions.

### Examples:
- Running out of memory
- Missing required file
- Invalid user input

---

## 🛠 What is Exception Handling?
**Exception handling** is the mechanism to detect and respond to runtime errors, ensuring the program can either recover gracefully or exit cleanly.

- Handle the error without crashing
- Warn the user about the error
- Preserve or save user data

---

## ⚠ Types of Errors
| Type          | Description |
|---------------|-------------|
| Compile Error | Caused by syntax mistakes — easy to fix, happens before execution |
| Runtime Error | Caused by logic mistakes, invalid input, or unexpected situations during execution |

---

## 🔹 Basic Syntax in C++
```cpp
try {
    // Code that may throw
    if (error) throw e;
} 
catch (exception_type e) {
    // Handle the exception
}
```

---

## 📄 Example
```cpp
int main() {
    int a, b;
    std::cout << "Input 2 numbers: ";
    std::cin >> a >> b;
    try {
        if (b == 0) throw b;
        std::cout << "a / b = " << a / b << std::endl;
        std::cout << "a % b = " << a % b << std::endl;
    } catch (int exception) {
        std::cout << "Exception: " << exception << std::endl;
    }
    std::cout << "Finished" << std::endl;
    return 0;
}
```

**Output:**
```
Input 2 numbers: 1 0
Exception: 0
Finished
```

---

## 🔄 Exceptions Can Propagate
Exceptions can be thrown inside a function and caught in the caller.

```cpp
int divide(int a, int b) {
    if (b == 0) throw b;
    return a / b;
}

int main() {
    try {
        std::cout << divide(1, 0) << std::endl;
    } catch (int e) {
        std::cout << "Exception: " << e << std::endl;
    }
}
```

---

## 🎯 Matching Exception Types
```cpp
try { throw 3; }
catch (int e) { std::cout << "Int: " << e << std::endl; }

try { throw 3.14; }
catch (double e) { std::cout << "Double: " << e << std::endl; }

try { throw "Error"; }
catch (const char* e) { std::cout << "String: " << e << std::endl; }
```

---

## 🔀 Nested try-catch
```cpp
try {
    try {
        throw "abc";
    } catch (const char* e) {
        std::cout << "Inner: " << e << std::endl;
    }
    throw 3;
} catch (int e) {
    std::cout << "Outer: " << e << std::endl;
}
```

---

## 📌 Function Exception Specification (Legacy)
> **Note:** `throw()` specifications are deprecated in C++11 and removed in C++17.

```cpp
double valueAt(double* p, int index) throw(int, const char*) {
    if (index < 0) throw "Out of range";
    else if (!p) throw 0;
    return p[index];
}
```

---

## 🏗 Custom Exception Class
```cpp
class FileNotFoundException : public std::exception {
    std::string message;
public:
    explicit FileNotFoundException(const std::string& file)
        : message("File not found: " + file) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
};
```

Usage:
```cpp
try {
    throw FileNotFoundException("Sample.dat");
} catch (std::exception& e) {
    std::cout << e.what() << std::endl;
}
```

---

## 📚 Summary Table
| Keyword / Concept | Description |
|-------------------|-------------|
| try               | Defines a block where exceptions may occur |
| throw             | Generates an exception |
| catch             | Handles an exception |
| std::exception    | Base class for standard exceptions |
| Custom exception  | User-defined exception type |

---

## ✅ Best Practices
- Catch exceptions by **const reference** to avoid slicing
- Use `std::exception` hierarchy where possible
- Avoid using exceptions for control flow
- Prefer RAII to manage resources and avoid leaks
