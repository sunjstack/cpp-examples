#include <utility> // for std::move and std::exchange
#include <cstddef> // for std::nullptr_t
#include <memory>  // for std::default_delete

// Custom deleter base template
template<typename T>
struct DefaultDeleter {
    void operator()(T* ptr) const {
        delete ptr;
    }
};

// Specialization for arrays
template<typename T>
struct DefaultDeleter<T[]> {
    void operator()(T* ptr) const {
        delete[] ptr;
    }
};

template<typename T, typename Deleter = DefaultDeleter<T>>
class MyUniquePtr {
private:
    T* ptr;
    Deleter deleter;

public:
    // Default constructor
    MyUniquePtr() : ptr(nullptr) {}

    // Constructor that takes a raw pointer
    explicit MyUniquePtr(T* p) : ptr(p) {}

    // Constructor that takes a raw pointer and custom deleter
    MyUniquePtr(T* p, Deleter d) : ptr(p), deleter(d) {}

    // Delete copy constructor and copy assignment to enforce unique ownership
    MyUniquePtr(const MyUniquePtr&) = delete;
    MyUniquePtr& operator=(const MyUniquePtr&) = delete;

    // Move constructor
    MyUniquePtr(MyUniquePtr&& other) noexcept : ptr(other.ptr), deleter(std::move(other.deleter)) {
        other.ptr = nullptr;
    }

    // Move assignment operator
    MyUniquePtr& operator=(MyUniquePtr&& other) noexcept {
        if (this != &other) {
            reset();
            ptr = other.ptr;
            deleter = std::move(other.deleter);
            other.ptr = nullptr;
        }
        return *this;
    }

    // Destructor
    ~MyUniquePtr() {
        reset();
    }

    // Overload dereference operator (only for non-array types)
    T& operator*() const requires (!std::is_array_v<T>) {
        return *ptr;
    }

    // Overload arrow operator (only for non-array types)
    T* operator->() const requires (!std::is_array_v<T>) {
        return ptr;
    }

    // Return the raw pointer
    T* get() const {
        return ptr;
    }

    // Release the ownership of the pointer and return it
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }

    // Replace the managed object
    void reset(T* p = nullptr) {
        if (ptr) {
            deleter(ptr);
        }
        ptr = p;
    }

    // Swap pointers with another MyUniquePtr
    void swap(MyUniquePtr& other) noexcept {
        std::swap(ptr, other.ptr);
        std::swap(deleter, other.deleter);
    }
};

// Non-member swap function
template<typename T, typename Deleter>
void swap(MyUniquePtr<T, Deleter>& lhs, MyUniquePtr<T, Deleter>& rhs) noexcept {
    lhs.swap(rhs);
}

// Utility function to create a MyUniquePtr (non-array)
template<typename T, typename... Args>
MyUniquePtr<T> make_unique(Args&&... args) {
    return MyUniquePtr<T>(new T(std::forward<Args>(args)...));
}

// Utility function to create a MyUniquePtr (array)
template<typename T>
MyUniquePtr<T[]> make_unique(std::size_t size) {
    return MyUniquePtr<T[]>(new T[size]());
}


#include <iostream>

int main() {
    // MyUniquePtr for a single object
    MyUniquePtr<int> ptr1 = make_unique<int>(42);
    std::cout << *ptr1 << std::endl;  // Output: 42

    // MyUniquePtr for an array
    MyUniquePtr<int[]> arrPtr = make_unique<int[]>(5);
    for (int i = 0; i < 5; ++i) {
        arrPtr.get()[i] = i * 10;
        std::cout << arrPtr.get()[i] << " ";  // Output: 0 10 20 30 40
    }
    std::cout << std::endl;

    // Custom deleter example
    MyUniquePtr<int, void(*)(int*)> ptr2(new int(100), [](int* p) {
        std::cout << "Custom delete: " << *p << std::endl;
        delete p;
        });
    std::cout << *ptr2 << std::endl;  // Output: 100

    return 0;
}
