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

#include "runtime_trampolines.h"
#include "ecmascript/accessor_data.h"
#include "ecmascript/ecma_macros.h"
#include "ecmascript/frames.h"
#include "ecmascript/interpreter/interpreter-inl.h"
#include "ecmascript/js_object.h"
#include "ecmascript/js_proxy.h"
#include "ecmascript/layout_info.h"
#include "ecmascript/message_string.h"
#include "ecmascript/object_factory.h"
#include "ecmascript/tagged_dictionary.h"

namespace panda::ecmascript {
bool RuntimeTrampolines::AddElementInternal(uint64_t argThread, uint64_t argReceiver, uint32_t argIndex,
                                            uint64_t argValue, uint32_t argAttr)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSObject> receiver(thread, reinterpret_cast<TaggedObject *>(argReceiver));
    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argValue)));
    auto attr = static_cast<PropertyAttributes>(argAttr);
    return JSObject::AddElementInternal(thread, receiver, argIndex, value, attr);
}

bool RuntimeTrampolines::CallSetter(uint64_t argThread, uint64_t argSetter, uint64_t argReceiver, uint64_t argValue,
                                    bool argMayThrow)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t * prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    JSHandle<JSTaggedValue> receiver(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argReceiver)));
    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argValue)));
    auto setter = AccessorData::Cast((reinterpret_cast<panda::ObjectHeader *>(argSetter)));
    return JSObject::CallSetter(thread, *setter, receiver, value, argMayThrow);
}

void RuntimeTrampolines::ThrowTypeError(uint64_t argThread, int argMessageStringId)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    std::string message = MessageString::GetMessageString(argMessageStringId);
    ObjectFactory *factory = JSThread::Cast(thread)->GetEcmaVM()->GetFactory();
    JSHandle<JSObject> error = factory->GetJSError(ErrorType::TYPE_ERROR, message.c_str());
    THROW_NEW_ERROR_AND_RETURN(thread, error.GetTaggedValue());
}

bool RuntimeTrampolines::JSProxySetProperty(uint64_t argThread, uint64_t argProxy, uint64_t argKey, uint64_t argValue,
                                            uint64_t argReceiver, bool argMayThrow)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    [[maybe_unused]] EcmaHandleScope handleScope(thread);
    JSHandle<JSProxy> proxy(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argProxy)));
    JSHandle<JSTaggedValue> index(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argKey)));
    JSHandle<JSTaggedValue> value(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argValue)));
    JSHandle<JSTaggedValue> receiver(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argReceiver)));

    return JSProxy::SetProperty(thread, proxy, index, value, receiver, argMayThrow);
}

uint32_t RuntimeTrampolines::GetHash32(uint64_t key, uint64_t len)
{
    auto pkey = reinterpret_cast<uint8_t *>(key);
    return panda::GetHash32(pkey, static_cast<size_t>(len));
}

uint64_t RuntimeTrampolines::CallGetter(uint64_t argThread, uint64_t argGetter, uint64_t argReceiver)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    auto accessor = AccessorData::Cast(reinterpret_cast<TaggedObject *>(argGetter));
    JSHandle<JSTaggedValue> objHandle(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argReceiver)));
    return JSObject::CallGetter(thread, accessor, objHandle).GetRawData();
}

uint64_t RuntimeTrampolines::AccessorGetter(uint64_t argThread, uint64_t argGetter, uint64_t argReceiver)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    auto accessor = AccessorData::Cast(reinterpret_cast<TaggedObject *>(argGetter));
    JSHandle<JSObject> objHandle(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argReceiver)));
    return accessor->CallInternalGet(thread, objHandle).GetRawData();
}

int32_t RuntimeTrampolines::FindElementWithCache(uint64_t argThread, uint64_t hClass, uint64_t key, int32_t num)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    auto cls  = reinterpret_cast<JSHClass *>(hClass);
    auto layoutInfo = LayoutInfo::Cast(cls->GetAttributes().GetTaggedObject());
    return layoutInfo->FindElementWithCache(thread, cls, JSTaggedValue(key), num);
}

uint32_t RuntimeTrampolines::StringGetHashCode(uint64_t ecmaString)
{
    auto string = reinterpret_cast<EcmaString *>(ecmaString);
    return string->GetHashcode();
}

void RuntimeTrampolines::PrintHeapReginInfo(uint64_t argThread)
{
    auto thread = reinterpret_cast<JSThread *>(argThread);
    thread->GetEcmaVM()->GetHeap()->GetNewSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "semispace region: " << current << std::endl;
    });
    thread->GetEcmaVM()->GetHeap()->GetOldSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "GetOldSpace region: " << current << std::endl;
    });
    thread->GetEcmaVM()->GetHeap()->GetNonMovableSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "GetNonMovableSpace region: " << current << std::endl;
    });
    thread->GetEcmaVM()->GetHeap()->GetMachineCodeSpace()->EnumerateRegions([](Region *current) {
        LOG_ECMA(INFO) << "GetMachineCodeSpace region: " << current << std::endl;
    });
}

TaggedArray* RuntimeTrampolines::GetTaggedArrayPtrTest(uint64_t argThread)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    // this case static static JSHandle<TaggedArray> arr don't free in first call
    // second call trigger gc.
    // don't call EcmaHandleScope handleScope(thread);
    ObjectFactory *factory = thread->GetEcmaVM()->GetFactory();
    static int i = 0;
    static JSHandle<TaggedArray> arr = factory->NewTaggedArray(2);
    if (i == 0) {
        arr->Set(thread, 0, JSTaggedValue(3)); // 3: first element
        arr->Set(thread, 1, JSTaggedValue(4)); // 4: second element
    }
#ifndef NDEBUG
    PrintHeapReginInfo(argThread);
#endif
    if (i != 0) {
        thread->GetEcmaVM()->CollectGarbage(TriggerGCType::COMPRESS_FULL_GC);
    }
    LOG_ECMA(INFO) << " arr->GetData() " << std::hex << "  " << arr->GetData();
    i++;
    return *arr;
}
uint64_t RuntimeTrampolines::Execute(uint64_t argThread, uint64_t argFunc,
                                     uint64_t thisArg, uint32_t argc, uint64_t argArgv)
{
    auto thread = reinterpret_cast<JSThread *>(argThread);
    auto func = reinterpret_cast<ECMAObject *>(argFunc);
    auto argv = reinterpret_cast<const TaggedType *>(argArgv);
    CallParams params;
    params.callTarget = func;
    params.newTarget = JSTaggedValue::VALUE_UNDEFINED;
    params.thisArg = thisArg;
    params.argc = argc;
    params.argv = argv;

    return EcmaInterpreter::Execute(thread, params).GetRawData();
}

void RuntimeTrampolines::SetValueWithBarrier(uint64_t argThread, uint64_t argAddr, uint64_t argOffset,
                                             uint64_t argValue)
{
    auto thread = reinterpret_cast<const JSThread *>(argThread);
    auto addr = reinterpret_cast<void *>(argAddr);
    auto offset = static_cast<size_t>(argOffset);
    auto value = static_cast<JSTaggedValue>(argValue);

    SET_VALUE_WITH_BARRIER(thread, addr, offset, value);
}

double RuntimeTrampolines::FloatMod(double left, double right)
{
    return std::fmod(left, right);
}

uint64_t RuntimeTrampolines::NewInternalString(uint64_t argThread, uint64_t argKey)
{
    uintptr_t *curFp = nullptr;
    auto thread = reinterpret_cast<JSThread *>(argThread);
    GET_CURRETN_FP(curFp);
    uintptr_t *prevFp = GET_PREV_FP(curFp);
    CallRuntimeTrampolinesScope scope(thread, prevFp, curFp);
    JSHandle<JSTaggedValue> keyHandle(thread, JSTaggedValue(reinterpret_cast<TaggedObject *>(argKey)));
    return JSTaggedValue(thread->GetEcmaVM()->GetFactory()->InternString(keyHandle)).GetRawData();
}
}  // namespace panda::ecmascript
