/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECMASCRIPT_TAGGED_QUEUE_H
#define ECMASCRIPT_TAGGED_QUEUE_H

#include "ecmascript/js_tagged_value.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/tagged_array.h"
#include "ecmascript/tagged_array-inl.h"

namespace panda::ecmascript {
class TaggedQueue : public TaggedArray {
public:
    static TaggedQueue *Cast(ObjectHeader *object)
    {
        return reinterpret_cast<TaggedQueue *>(object);
    }

    JSTaggedValue Pop(JSThread *thread);
    static TaggedQueue *Push(const JSThread *thread, const JSHandle<TaggedQueue> &queue,
                             const JSHandle<JSTaggedValue> &value)
    {
        array_size_t capacity = queue->GetCapacity().GetArrayLength();
        if (capacity == 0) {
            // If there is no capacity, directly create a queue whose capacity is MIN_CAPACITY. Add elements.
            ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
            JSHandle<TaggedQueue> newQueue = factory->NewTaggedQueue(MIN_CAPACITY);
            newQueue->Set(thread, 0, value.GetTaggedValue());
            newQueue->SetCapacity(thread, JSTaggedValue(MIN_CAPACITY));
            newQueue->SetStart(thread, JSTaggedValue(0));
            newQueue->SetEnd(thread, JSTaggedValue(1));
            return *newQueue;
        }

        array_size_t start = queue->GetStart().GetArrayLength();
        array_size_t end = queue->GetEnd().GetArrayLength();
        array_size_t size = queue->Size();
        if ((end + 1) % capacity == start) {
            // The original queue is full and needs to be expanded.
            if (capacity == MAX_QUEUE_INDEX) {
                // can't grow anymore
                return *queue;
            }
            ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
            // Grow Array for 1.5 times
            array_size_t newCapacity = capacity + (capacity >> 1U);
            newCapacity = newCapacity < capacity ? MAX_QUEUE_INDEX : newCapacity;
            JSHandle<TaggedQueue> newQueue = factory->NewTaggedQueue(newCapacity);
            array_size_t newEnd = 0;
            for (array_size_t i = start; newEnd < size; i = (i + 1) % capacity) {
                newQueue->Set(thread, newEnd, queue->Get(i));
                newEnd++;
            }

            newQueue->SetCapacity(thread, JSTaggedValue(newCapacity));
            newQueue->SetStart(thread, JSTaggedValue(0));
            newQueue->Set(thread, newEnd++, value.GetTaggedValue());
            newQueue->SetEnd(thread, JSTaggedValue(newEnd));
            return *newQueue;
        }
        queue->Set(thread, end, value.GetTaggedValue());
        queue->SetEnd(thread, JSTaggedValue((end + 1) % capacity));
        return *queue;
    }

    // Only for fixed-length queue, without grow capacity
    static inline void PushFixedQueue(const JSThread *thread, const JSHandle<TaggedQueue> &queue,
                                      const JSHandle<JSTaggedValue> &value)
    {
        array_size_t end = queue->GetEnd().GetArrayLength();
        array_size_t capacity = queue->GetCapacity().GetArrayLength();
        ASSERT(capacity != 0);
        queue->Set(thread, end, value.GetTaggedValue());
        queue->SetEnd(thread, JSTaggedValue((end + 1) % capacity));
    }

    inline bool Empty()
    {
        return GetStart() == GetEnd();
    }

    inline JSTaggedValue Front()
    {
        if (Empty()) {
            return JSTaggedValue::Hole();
        }
        array_size_t start = GetStart().GetArrayLength();
        return JSTaggedValue(Get(start));
    }

    inline JSTaggedValue Back()
    {
        if (Empty()) {
            return JSTaggedValue::Hole();
        }
        return JSTaggedValue(Get(GetEnd().GetArrayLength() - 1));
    }

    inline array_size_t Size()
    {
        array_size_t capacity = GetCapacity().GetArrayLength();
        if (capacity == 0) {
            return 0;
        }
        array_size_t end = GetEnd().GetArrayLength();
        array_size_t start = GetStart().GetArrayLength();
        return (end - start + capacity) % capacity;
    }

    inline JSTaggedValue Get(array_size_t index) const
    {
        return TaggedArray::Get(QueueToArrayIndex(index));
    }

    inline void Set(const JSThread *thread, array_size_t index, JSTaggedValue value)
    {
        return TaggedArray::Set(thread, QueueToArrayIndex(index), value);
    }

    static const array_size_t MIN_CAPACITY = 2;
    static const array_size_t CAPACITY_INDEX = 0;
    static const array_size_t START_INDEX = 1;
    static const array_size_t END_INDEX = 2;
    static const array_size_t ELEMENTS_START_INDEX = 3;
    static const array_size_t MAX_QUEUE_INDEX = TaggedArray::MAX_ARRAY_INDEX - ELEMENTS_START_INDEX;

private:
    friend class ObjectFactory;

    inline static constexpr array_size_t QueueToArrayIndex(array_size_t index)
    {
        return index + ELEMENTS_START_INDEX;
    }

    inline void SetCapacity(const JSThread *thread, JSTaggedValue capacity)
    {
        TaggedArray::Set(thread, CAPACITY_INDEX, capacity);
    }

    inline JSTaggedValue GetCapacity() const
    {
        return TaggedArray::Get(CAPACITY_INDEX);
    }

    inline void SetStart(const JSThread *thread, JSTaggedValue start)
    {
        TaggedArray::Set(thread, START_INDEX, start);
    }

    inline JSTaggedValue GetStart() const
    {
        return TaggedArray::Get(START_INDEX);
    }

    inline void SetEnd(const JSThread *thread, JSTaggedValue end)
    {
        TaggedArray::Set(thread, END_INDEX, end);
    }

    inline JSTaggedValue GetEnd() const
    {
        return TaggedArray::Get(END_INDEX);
    }

    static TaggedQueue *Create(JSThread *thread, array_size_t capacity, JSTaggedValue initVal = JSTaggedValue::Hole());
};
}  // namespace panda::ecmascript
#endif  // ECMASCRIPT_TAGGED_QUEUE_H