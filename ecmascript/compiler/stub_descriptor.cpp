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

#include "ecmascript/compiler/stub_descriptor.h"

#include "llvm-c/Core.h"
#include "llvm/Support/Host.h"

namespace kungfu {
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define CALL_STUB_INIT_DESCRIPTOR(name)                              \
    class Stub##name##InterfaceDescriptor final {                    \
    public:                                                          \
        static void Initialize(StubDescriptor *descriptor); \
    };                                                               \
    void Stub##name##InterfaceDescriptor::Initialize(StubDescriptor *descriptor)
CALL_STUB_INIT_DESCRIPTOR(FastAdd)
{
    // 2 : 2 input parameters
    StubDescriptor fastAdd("FastAdd", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastAdd;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(FastSub)
{
    // 2 : 2 input parameters
    StubDescriptor fastSub("FastSub", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastSub;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(FastMul)
{
    // 2 : 2 input parameters
    StubDescriptor fastMul("FastMul", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastMul;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

#ifndef NDEBUG
CALL_STUB_INIT_DESCRIPTOR(FastMulGCTest)
{
    // 3 : 3 input parameters
    StubDescriptor fastMulGC("FastMulGCTest", 0, 3, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastMulGC;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}
#else
CALL_STUB_INIT_DESCRIPTOR(FastMulGCTest) {}
#endif

CALL_STUB_INIT_DESCRIPTOR(FastDiv)
{
    // 2 : 2 input parameters
    StubDescriptor fastDiv("FastDiv", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastDiv;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(FastMod)
{
    // 3 : 3 input parameters
    StubDescriptor fastMod("FastMod", 0, 3, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastMod;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(FloatMod)
{
    // 2 : 2 input parameters
    StubDescriptor floatMod("FloatMod", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::FLOAT64_TYPE);
    *descriptor = floatMod;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::FLOAT64_TYPE,
        MachineType::FLOAT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(FastTypeOf)
{
    // 2 input parameters
    StubDescriptor fastTypeOf("FastTypeOf", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastTypeOf;
    // 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(FastEqual)
{
    // 2 input parameters
    StubDescriptor fastEqual("FastEqual", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = fastEqual;
    // 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(FastStrictEqual) {}

CALL_STUB_INIT_DESCRIPTOR(IsSpecialIndexedObjForSet) {}

CALL_STUB_INIT_DESCRIPTOR(IsSpecialIndexedObjForGet) {}

CALL_STUB_INIT_DESCRIPTOR(GetElement)
{
    // 3 : 3 input parameters
    StubDescriptor getElement("GetElement", 0, 3, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = getElement;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(SetElement)
{
    // 5 : 5 input parameters
    StubDescriptor setElement("SetElement", 0, 5, ArgumentsOrder::DEFAULT_ORDER, MachineType::BOOL_TYPE);
    *descriptor = setElement;
    // 5 : 5 input parameters
    std::array<MachineType, 5> params = {
        MachineType::UINT64_TYPE, MachineType::UINT64_TYPE, MachineType::UINT32_TYPE,
        MachineType::UINT64_TYPE, MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(SetPropertyByName) {}

CALL_STUB_INIT_DESCRIPTOR(GetPropertyByName)
{
        // 3 : 3 input parameters
    StubDescriptor getPropertyByName("GetPropertyByName", 0, 3, ArgumentsOrder::DEFAULT_ORDER,
        MachineType::UINT64_TYPE);
    *descriptor = getPropertyByName;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE, MachineType::UINT64_TYPE, MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(SetGlobalOwnProperty) {}

CALL_STUB_INIT_DESCRIPTOR(GetGlobalOwnProperty) {}

CALL_STUB_INIT_DESCRIPTOR(SetOwnPropertyByName) {}

CALL_STUB_INIT_DESCRIPTOR(SetOwnElement) {}

CALL_STUB_INIT_DESCRIPTOR(FastSetProperty) {}

CALL_STUB_INIT_DESCRIPTOR(FastGetProperty) {}

CALL_STUB_INIT_DESCRIPTOR(FindOwnProperty) {}

CALL_STUB_INIT_DESCRIPTOR(FindOwnElement)
{
    // 3 : 3 input parameters
    StubDescriptor findOwnElement("FindOwnElement", 0, 3, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = findOwnElement;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(NewLexicalEnvDyn) {}

CALL_STUB_INIT_DESCRIPTOR(FindOwnProperty2) {}

CALL_STUB_INIT_DESCRIPTOR(FindOwnElement2)
{
    // 6 : 6 input parameters
    StubDescriptor findOwnElement2("FindOwnElement2", 0, 6, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = findOwnElement2;
    // 6 : 6 input parameters
    std::array<MachineType, 6> params = {
        MachineType::UINT64_TYPE, MachineType::UINT64_TYPE, MachineType::UINT32_TYPE,
        MachineType::BOOL_TYPE,   MachineType::UINT64_TYPE, MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(GetPropertyByIndex)
{
    // 3 : 3 input parameters
    StubDescriptor getPropertyByIndex("GetPropertyByIndex", 0, 3, ArgumentsOrder::DEFAULT_ORDER,
        MachineType::UINT64_TYPE);
    *descriptor = getPropertyByIndex;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(SetPropertyByIndex)
{
    // 4 : 4 input parameters
    StubDescriptor setPropertyByIndex("SetPropertyByIndex", 0, 4, ArgumentsOrder::DEFAULT_ORDER,
        MachineType::UINT64_TYPE);
    *descriptor = setPropertyByIndex;
    // 4 : 4 input parameters
    std::array<MachineType, 4> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(GetPropertyByValue)
{
    // 3 : 3 input parameters
    StubDescriptor getPropertyByValue("GetPropertyByValue", 0, 3,
        ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = getPropertyByValue;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
}

CALL_STUB_INIT_DESCRIPTOR(SetPropertyByValue)
{
}


CALL_STUB_INIT_DESCRIPTOR(AddElementInternal)
{
    // 5 : 5 input parameters
    StubDescriptor addElementInternal("AddElementInternal", 0, 5, ArgumentsOrder::DEFAULT_ORDER,
        MachineType::BOOL_TYPE);
    *descriptor = addElementInternal;
    // 5 : 5 input parameters
    std::array<MachineType, 5> params = {
        MachineType::UINT64_TYPE, MachineType::UINT64_TYPE, MachineType::UINT32_TYPE,
        MachineType::UINT64_TYPE, MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(GetTaggedArrayPtrTest)
{
    StubDescriptor getTaggedArrayPtr("GetTaggedArrayPtrTest", 0, 1, ArgumentsOrder::DEFAULT_ORDER,
                                             MachineType::TAGGED_POINTER_TYPE);
    *descriptor = getTaggedArrayPtr;
    std::array<MachineType, 1> params = {
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(CallSetter)
{
    // 5 : 5 input parameters
    StubDescriptor callSetter("CallSetter", 0, 5, ArgumentsOrder::DEFAULT_ORDER, MachineType::BOOL_TYPE);
    *descriptor = callSetter;
    // 5 : 5 input parameters
    std::array<MachineType, 5> params = {
        MachineType::UINT64_TYPE, MachineType::UINT64_TYPE, MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE, MachineType::BOOL_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(CallGetter)
{
    // 3 : 3 input parameters
    StubDescriptor callGetter("CallGetter", 0, 3, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = callGetter;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(AccessorGetter)
{
    // 3 : 3 input parameters
    StubDescriptor accessorGetter("AccessorGetter", 0, 3, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = accessorGetter;
    // 3 : 3 input parameters
    std::array<MachineType, 3> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(ThrowTypeError)
{
    // 2 : 2 input parameters
    StubDescriptor throwTypeError("ThrowTypeError", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::BOOL_TYPE);
    *descriptor = throwTypeError;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(JSProxySetProperty)
{
    // 6 : 6 input parameters
    StubDescriptor jsproxySetproperty("JSProxySetProperty", 0, 6,
        ArgumentsOrder::DEFAULT_ORDER, MachineType::BOOL_TYPE);
    *descriptor = jsproxySetproperty;
    // 6 : 6 input parameters
    std::array<MachineType, 6> params = {
        MachineType::UINT64_TYPE, MachineType::UINT64_TYPE, MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE, MachineType::UINT64_TYPE, MachineType::BOOL_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(GetHash32)
{
    // 2 : 2 input parameters
    StubDescriptor getHash32("GetHash32", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT32_TYPE);
    *descriptor = getHash32;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(FindElementWithCache)
{
    // 4 : 4 input parameters
    StubDescriptor findElementWithCache("FindElementWithCache", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, MachineType::INT32_TYPE);
    *descriptor = findElementWithCache;
    std::array<MachineType, 4> params = {  // 4 : 4 input parameters
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(Execute)
{
    // 5 : 5 input parameters
    StubDescriptor execute("Execute", 0, 5, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = execute;
    std::array<MachineType, 5> params = {  // 5 : 5 input parameters
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(FunctionCallInternal)
{
    // 5 : 5 input parameters
    StubDescriptor functionCallInternal("FunctionCallInternal", 0, 5,
                                               ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = functionCallInternal;
    std::array<MachineType, 5> params = {  // 5 : 5 input parameters
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(StringGetHashCode)
{
    StubDescriptor stringGetHashCode("StringGetHashCode", 0, 1,
        ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT32_TYPE);
    *descriptor = stringGetHashCode;
    std::array<MachineType, 1> params = {
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(SetValueWithBarrier)
{
    // 4 : 4 input parameters
    static StubDescriptor SetValueWithBarrier("SetValueWithBarrier", 0, 4,
        ArgumentsOrder::DEFAULT_ORDER, MachineType::NONE_TYPE);
    *descriptor = SetValueWithBarrier;
    // 4 : 4 input parameters
    std::array<MachineType, 4> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(NewInternalString)
{
    // 2 : 2 input parameters
    StubDescriptor stringGetHashCode("NewInternalString", 0, 2,
        ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT64_TYPE);
    *descriptor = stringGetHashCode;
    // 2 : 2 input parameters
    std::array<MachineType, 2> params = {
        MachineType::UINT64_TYPE,
        MachineType::UINT64_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::RUNTIME_STUB);
}

CALL_STUB_INIT_DESCRIPTOR(FastLoadElement)
{
    // 2 : 2 input parameters
    StubDescriptor fastLoadElement("FastLoadElement", 0, 2, ArgumentsOrder::DEFAULT_ORDER, MachineType::TAGGED_TYPE);
    *descriptor = fastLoadElement;
    std::array<MachineType, 2> params = { // 2 : 2 input parameters
        MachineType::UINT64_TYPE,
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::TEST_FUNC);
}

CALL_STUB_INIT_DESCRIPTOR(PhiGateTest)
{
    StubDescriptor phiGateTest("PhiGateTest", 0, 1, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT32_TYPE);
    *descriptor = phiGateTest;
    std::array<MachineType, 1> params = {
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::TEST_FUNC);
}

CALL_STUB_INIT_DESCRIPTOR(LoopTest)
{
    StubDescriptor loopTest("LoopTest", 0, 1, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT32_TYPE);
    *descriptor = loopTest;
    std::array<MachineType, 1> params = {
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::TEST_FUNC);
}

CALL_STUB_INIT_DESCRIPTOR(LoopTest1)
{
    StubDescriptor loopTest1("LoopTest1", 0, 1, ArgumentsOrder::DEFAULT_ORDER, MachineType::UINT32_TYPE);
    *descriptor = loopTest1;
    std::array<MachineType, 1> params = {
        MachineType::UINT32_TYPE,
    };
    descriptor->SetParameters(params.data());
    descriptor->SetStubKind(StubDescriptor::CallStubKind::TEST_FUNC);
}

void FastStubDescriptors::InitializeStubDescriptors()
{
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEF_CALL_STUB(name) NAME_##name
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define INITIALIZE_CALL_STUB_DESCRIPTOR(name, argcounts) \
    Stub##name##InterfaceDescriptor::Initialize(&callStubsDescriptor_[DEF_CALL_STUB(name)]);
    CALL_STUB_LIST(INITIALIZE_CALL_STUB_DESCRIPTOR)
#undef INITIALIZE_CALL_STUB_DESCRIPTOR
#undef DEF_CALL_STUB
}
}  // namespace kungfu
