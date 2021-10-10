//
//  ut_SharedPtr.cpp
//  SharedPtr
//
//  Created by Mikhail Gorshkov on 09.10.2021.
//

#include <gtest/gtest.h>
#include <thread>

#include "SharedPtr.hpp"

class SharedPtrTest : public ::testing::Test {

  protected:
    template <typename T>
    class Mock {
    public:
        explicit Mock(const T& object)
            : object{object}{
        }
        
        void operator delete (void* ptr) {
            deleteCalled = true;
            ::operator delete(ptr);
        }
        ~Mock() {
            destructorCalled = true;
        }
        
        T object;

        static bool destructorCalled;
        static bool deleteCalled;
        
        static bool destroyedOk() {
            return destructorCalled && deleteCalled;
        }
    };

    
    using MockInt = Mock<int>;
    using MockDouble = Mock<double>;

    void SetUp() {
        MockInt::destructorCalled = false;
        MockDouble::destructorCalled = false;
    }

    void TearDown() {
      
    }
};

template <typename T>
bool SharedPtrTest::Mock<T>::destructorCalled = false;

template <typename T>
bool SharedPtrTest::Mock<T>::deleteCalled = false;


TEST_F(SharedPtrTest, testDefault)
{
    SharedPtr<int> ptr;
    // no crash, test has passed
}

TEST_F(SharedPtrTest, testSharedPtrDeletesObject)
{
    {
        SharedPtr<MockInt> sharedPtr{new MockInt{1}};
        EXPECT_FALSE(MockInt::destroyedOk());
    }
    EXPECT_TRUE(MockInt::destroyedOk());
}

TEST_F(SharedPtrTest, testSharedPtrCopy)
{
    {
        SharedPtr<MockInt> sharedPtr1{new MockInt{1}};
        {
            auto sharedPtr2{sharedPtr1};
            EXPECT_FALSE(MockInt::destroyedOk());
        }
        EXPECT_FALSE(MockInt::destroyedOk());
    }
    EXPECT_TRUE(MockInt::destroyedOk());
}

TEST_F(SharedPtrTest, testSharedPtrDereference)
{
    auto ptr = new int{12345};
    SharedPtr<int> sharedPtr{ptr};
    EXPECT_EQ(12345, *sharedPtr);   // const dereference
    *sharedPtr = 67890;             // non-const dereference
    EXPECT_EQ(67890, *sharedPtr);
    EXPECT_EQ(67890, *ptr);
}

TEST_F(SharedPtrTest, testSharedPtrReset)
{
    auto ptr = new int{12345};
    SharedPtr<int> sharedPtr{ptr};
    SharedPtr<int> sharedPtr2{sharedPtr};
    sharedPtr2.reset(new int{67890});
    EXPECT_EQ(67890, *sharedPtr2);
    EXPECT_EQ(12345, *sharedPtr);
}

TEST_F(SharedPtrTest, testCountersMultiThreading)
{
    {
        SharedPtr<MockInt> sharedPtr1{new MockInt{12345}};
        SharedPtr<MockDouble> sharedPtr2{new MockDouble{67890.12345}};
        
        std::vector<std::thread> threads;
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back([sharedPtr1, sharedPtr2](int num){
                if (num % 2 == 0) {
                    SharedPtr<MockInt> v1[100];
                    for (int i = 0; i < 100; ++i) {
                        v1[i] = sharedPtr1;
                        usleep(100);
                    }
                } else {
                    SharedPtr<MockDouble> v2[100];
                    for (int i = 0; i < 100; ++i) {
                        v2[i] = sharedPtr2;
                        usleep(100);
                    }
                }
            }, i);
        }
        for (auto& thread : threads) {
            thread.join();
        }
        
        EXPECT_FALSE(MockInt::destroyedOk());
        EXPECT_FALSE(MockDouble::destroyedOk());
    }
    EXPECT_TRUE(MockInt::destroyedOk());
    EXPECT_TRUE(MockDouble::destroyedOk());
}
