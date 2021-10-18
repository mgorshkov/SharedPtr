//
//  SharedPtr.hpp
//  SharedPtr
//
//  Created by Mikhail Gorshkov on 09.10.2021.
//

#ifndef SharedPtr_hpp
#define SharedPtr_hpp

#include <atomic>

// A simple implementation of Shared Pointer
// Not thread-safe
template <typename T>
class SharedPtr {
public:
    // Default constructor
    SharedPtr() noexcept {
        
    }
    
    // Copy constructor
    SharedPtr(const SharedPtr& another) noexcept {
        if (another.m_control_block != nullptr) {
            copy_from(another);
        }
    }
    
    // Initializing constructor. Once constructed, ptr is owned by the SharedPtr
    // @param ptr - non-null pointer to acquire ownership of
    explicit SharedPtr(T* ptr)
    {
        acquire(ptr);
    }
    
    // Assignment operator
    SharedPtr& operator = (const SharedPtr& another) noexcept
    {
        if (this != &another) {
            release();
            copy_from(another);
        }
        return *this;
    }
    
    // Destructor
    ~SharedPtr() noexcept {
        release();
    }
    
    // Dereference operator
    T& operator * () noexcept {
        return **m_control_block;
    }
    
    // Reset current ownership and acquire a new one of ptr
    // @param ptr - non-null pointer to acquire ownership of
    void reset(T* ptr) {
        release();
        acquire(ptr);
    }
    
private:
    class ControlBlock {
    public:
        ControlBlock() noexcept {
            
        }
        
        explicit ControlBlock(T* ptr) noexcept
            : m_ptr{ptr}
            , m_counter{1L}
        {
            
        }
        
        bool dec() noexcept {
            if (0 == --m_counter) {
                delete m_ptr;
                return true;
            }
            return false;
        }
        
        void inc() noexcept {
            ++m_counter;
        }
        
        T& operator * () noexcept {
            return *m_ptr;
        }
        
    private:
        T* m_ptr = nullptr;
        std::atomic_long m_counter;
    };
    
    void acquire(T* ptr) {
        assert(ptr != nullptr);
        m_control_block = new ControlBlock{ptr};
    }
    
    void release() noexcept {
        if (m_control_block != nullptr && m_control_block->dec())
            delete m_control_block;
    }
    
    void copy_from(const SharedPtr<T>& another) noexcept {
        m_control_block = another.m_control_block;
        m_control_block->inc();
    }
    
    ControlBlock* m_control_block = nullptr;
};
#endif /* SharedPtr_hpp */
