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

#include "ecmascript/global_env.h"
#include "ecmascript/js_array_iterator.h"
#include "ecmascript/js_array.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tests/test_helper.h"

using namespace panda::ecmascript;

namespace panda::test {
class JSArrayIteratorTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
        GTEST_LOG_(INFO) << "SetUpTestCase";
    }

    static void TearDownTestCase()
    {
        GTEST_LOG_(INFO) << "TearDownCase";
    }

    void SetUp() override
    {
        TestHelper::CreateEcmaVMWithScope(instance, thread, scope);
    }

    void TearDown() override
    {
        TestHelper::DestroyEcmaVMWithScope(instance, scope);
    }
    PandaVM *instance {nullptr};
    ecmascript::EcmaHandleScope *scope {nullptr};
    JSThread *thread {nullptr};
};

/*
 * Feature: JSArrayIterator
 * Function: SetIteratedArray
 * SubFunction: GetIteratedArray
 * FunctionPoints: Set Iterated Array
 * CaseDescription: Call the "SetIteratedArray" function, check whether the result returned through "GetIteratedArray"
 *                  function from the JSArrayIterator is within expectations.
 */
HWTEST_F_L0(JSArrayIteratorTest, SetIteratedArray)
{
    EcmaVM *ecmaVMPtr = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVMPtr->GetFactory();

    uint32_t arrayFrom1[10] = {0, 6, 8, 99, 200, 1, -1, -199, 33, 100};
    uint32_t arrayFrom2[10] = {1111, 3211, 737, 0, 1267, 174, 2763, 832, 11, 93};
    int numArrayFrom1 = sizeof(arrayFrom1)/sizeof(arrayFrom1[0]);
    int numArrayFrom2 = sizeof(arrayFrom2)/sizeof(arrayFrom2[0]);
    JSHandle<TaggedArray> handleTaggedArrayFrom1(factory->NewTaggedArray(numArrayFrom1));
    JSHandle<TaggedArray> handleTaggedArrayFrom2(factory->NewTaggedArray(numArrayFrom2));
    for (int i = 0; i < numArrayFrom1; i++) {
        handleTaggedArrayFrom1->Set(thread, i, JSTaggedValue(arrayFrom1[i]));
    }
    for (int i = 0; i < numArrayFrom2; i++) {
        handleTaggedArrayFrom2->Set(thread, i, JSTaggedValue(arrayFrom2[i]));
    }
    JSHandle<JSObject> handleJSObjectTaggedArrayFrom1(JSArray::CreateArrayFromList(thread, handleTaggedArrayFrom1));
    JSHandle<JSObject> handleJSObjectTaggedArrayFrom2(JSArray::CreateArrayFromList(thread, handleTaggedArrayFrom2));

    // Call "SetIteratedArray" function through "NewJSArrayIterator" function of "object_factory.cpp".
    JSHandle<JSArrayIterator> handleJSArrayIter = factory->NewJSArrayIterator(handleJSObjectTaggedArrayFrom1,
        IterationKind::KEY);

    JSHandle<JSArray> handleJSArrayTo1(thread, JSArray::Cast(handleJSArrayIter->GetIteratedArray().GetTaggedObject()));
    EXPECT_EQ(handleJSArrayTo1->GetArrayLength(), numArrayFrom1);
    for (int i = 0; i < numArrayFrom1; i++) {
        EXPECT_EQ(JSArray::FastGetPropertyByValue(thread, JSHandle<JSTaggedValue>(handleJSArrayTo1), i)->GetNumber(),
            arrayFrom1[i]);
    }

    // Call "SetIteratedArray" function in this HWTEST_F_L0.
    handleJSArrayIter->SetIteratedArray(thread, handleJSObjectTaggedArrayFrom2);

    JSHandle<JSArray> handleJSArrayTo2(thread, JSArray::Cast(handleJSArrayIter->GetIteratedArray().GetTaggedObject()));
    EXPECT_EQ(handleJSArrayTo2->GetArrayLength(), numArrayFrom2);
    for (int i = 0; i < numArrayFrom2; i++) {
        EXPECT_EQ(JSArray::FastGetPropertyByValue(thread, JSHandle<JSTaggedValue>(handleJSArrayTo2), i)->GetNumber(),
            arrayFrom2[i]);
    }
}

/*
 * Feature: JSArrayIterator
 * Function: SetNextIndex
 * SubFunction: GetNextIndex
 * FunctionPoints: Set Next Index
 * CaseDescription: Call the "SetNextIndex" function, check whether the result returned through "GetNextIndex" function
 *                  from the JSArrayIterator is within expectations.
 */
HWTEST_F_L0(JSArrayIteratorTest, SetNextIndex)
{
    EcmaVM *ecmaVMPtr = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVMPtr->GetFactory();

    uint32_t array[10] = {0, 6, 8, 99, 200, 1, -1, -199, 33, 100};
    int numArray = sizeof(array)/sizeof(array[0]);
    JSHandle<TaggedArray> handleTaggedArray(factory->NewTaggedArray(numArray));
    for (int i = 0; i < numArray; i++) {
        handleTaggedArray->Set(thread, i, JSTaggedValue(array[i]));
    }
    JSHandle<JSObject> handleJSObjectTaggedArray(JSArray::CreateArrayFromList(thread, handleTaggedArray));

    // Call "SetNextIndex" function through "NewJSArrayIterator" function of "object_factory.cpp".
    JSHandle<JSArrayIterator> handleJSArrayIter = factory->NewJSArrayIterator(handleJSObjectTaggedArray,
        IterationKind::KEY);
    EXPECT_EQ(handleJSArrayIter->GetNextIndex().GetNumber(), 0);

    int testQuantity = 100;
    for (int i = 1; i <= testQuantity; i++) {
        JSHandle<JSTaggedValue> handleTagValNextIndex(thread, JSTaggedValue(i));

        // Call "SetNextIndex" function in this HWTEST_F_L0.
        handleJSArrayIter->SetNextIndex(thread, handleTagValNextIndex);
        EXPECT_EQ(handleJSArrayIter->GetNextIndex().GetNumber(), i);
    }
}

/*
 * Feature: JSArrayIterator
 * Function: SetIterationKind
 * SubFunction: GetIterationKind
 * FunctionPoints: Set Iteration Kind
 * CaseDescription: Call the "SetIterationKind" function, check whether the result returned through "GetIterationKind"
 *                  function from the JSArrayIterator is within expectations.
 */
HWTEST_F_L0(JSArrayIteratorTest, SetIterationKind)
{
    EcmaVM *ecmaVMPtr = thread->GetEcmaVM();
    ObjectFactory *factory = ecmaVMPtr->GetFactory();

    uint32_t array[10] = {0, 6, 8, 99, 200, 1, -1, -199, 33, 100};
    int numArray = sizeof(array)/sizeof(array[0]);
    JSHandle<TaggedArray> handleTaggedArray(factory->NewTaggedArray(numArray));
    for (int i = 0; i < numArray; i++) {
        handleTaggedArray->Set(thread, i, JSTaggedValue(array[i]));
    }
    JSHandle<JSTaggedValue> handleTagVal0(thread, JSTaggedValue(0));
    JSHandle<JSTaggedValue> handleTagVal1(thread, JSTaggedValue(1));
    JSHandle<JSTaggedValue> handleTagVal2(thread, JSTaggedValue(2));
    JSHandle<JSObject> handleJSObjectTaggedArray(JSArray::CreateArrayFromList(thread, handleTaggedArray));

    // Call "SetIterationKind" function through "NewJSArrayIterator" function of "object_factory.cpp".
    JSHandle<JSArrayIterator> handleJSArrayIter = factory->NewJSArrayIterator(handleJSObjectTaggedArray,
        IterationKind::KEY);
    EXPECT_EQ(handleJSArrayIter->GetIterationKind().GetNumber(), 0);
    handleJSArrayIter = factory->NewJSArrayIterator(handleJSObjectTaggedArray, IterationKind::VALUE);
    EXPECT_EQ(handleJSArrayIter->GetIterationKind().GetNumber(), 1);
    handleJSArrayIter = factory->NewJSArrayIterator(handleJSObjectTaggedArray, IterationKind::KEY_AND_VALUE);
    EXPECT_EQ(handleJSArrayIter->GetIterationKind().GetNumber(), 2);

    // Call "SetIterationKind" function in this HWTEST_F_L0.
    handleJSArrayIter->SetIterationKind(thread, handleTagVal0);
    EXPECT_EQ(handleJSArrayIter->GetIterationKind().GetNumber(), 0);
    handleJSArrayIter->SetIterationKind(thread, handleTagVal1);
    EXPECT_EQ(handleJSArrayIter->GetIterationKind().GetNumber(), 1);
    handleJSArrayIter->SetIterationKind(thread, handleTagVal2);
    EXPECT_EQ(handleJSArrayIter->GetIterationKind().GetNumber(), 2);
}
}  // namespace panda::ecmascript
