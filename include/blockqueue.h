//
// Created by cambricon on 19-1-2.
//

#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <condition_variable>
#include <mutex>
#include <queue> // std::queue & std::priority_queue
#include <iostream>

template<typename PTData>
class BlockQueue
{
public:
    BlockQueue();
    BlockQueue(long long maxSize = -1);
    unsigned long long getMaxSize() const;
    bool push(PTData pdata);
    bool pop(PTData &pdata);
    bool push_non_block(PTData &pdata);
    bool pop_non_block(PTData &pdata);
    bool peek(PTData &pdata);
    int size() const;

private:
    mutable std::mutex mMutex;
    std::condition_variable mConditionVariable;
    std::queue<PTData> mTQueue;

private:
    const long long mMaxSize;
};


template<typename PTData>
BlockQueue<PTData>::BlockQueue(long long maxSize) :
        mMaxSize{maxSize}
{
}


template<typename PTData>
BlockQueue<PTData>::BlockQueue()
{
    mMaxSize = -1;
}

template<typename PTData>
unsigned long long BlockQueue<PTData>::getMaxSize() const
{
    return mMaxSize;
}

template<typename PTData>
bool BlockQueue<PTData>::push(PTData pdata)
{
    try
    {
        std::unique_lock<std::mutex> lock{mMutex};
        mConditionVariable.wait(lock, [this]{return mTQueue.size() < getMaxSize(); });
        mTQueue.push(pdata);
        mConditionVariable.notify_all();
        return true;
    }
    catch (const std::exception& e)
    {
//        error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        return false;
    }
}

template<typename PTData>
bool BlockQueue<PTData>::peek(PTData& pdata)
{
    try
    {
        std::unique_lock<std::mutex> lock{mMutex};
        mConditionVariable.wait(lock, [this]{return !mTQueue.empty();});

        pdata = this->mTQueue.front();
        this->mConditionVariable.notify_one();
        return true;
    }
    catch (const std::exception& e)
    {
        return false;
    }
}

template<typename PTData>
bool BlockQueue<PTData>::pop(PTData& pdata)
{
    try
    {
        std::unique_lock<std::mutex> lock{mMutex};
        mConditionVariable.wait(lock, [this]{return !mTQueue.empty();});

        pdata = this->mTQueue.front();
        this->mTQueue.pop();
        this->mConditionVariable.notify_one();
        return true;
    }
    catch (const std::exception& e)
    {
//        error(e.what(), __LINE__, __FUNCTION__, __FILE__);
        return false;
    }
}

template<typename PTData>
bool BlockQueue<PTData>::push_non_block(PTData &pdata)
{
    try
    {
        std::unique_lock<std::mutex> lock{mMutex};
        if(mTQueue.size() >= getMaxSize())
            return false;

        mTQueue.push(pdata);
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
}


template<typename PTData>
bool BlockQueue<PTData>::pop_non_block(PTData &pdata)
{
    try
    {
        std::unique_lock<std::mutex> lock{mMutex};
        if(mTQueue.empty()){
            return false;
        }
        pdata = this->mTQueue.front();
        this->mTQueue.pop();
        return true;
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
        return false;
    }
}


template<typename PTData>
int BlockQueue<PTData>::size() const {
    std::unique_lock<std::mutex> lock{mMutex};
    return mTQueue.size();
}

#endif //BLOCKQUEUE_H
