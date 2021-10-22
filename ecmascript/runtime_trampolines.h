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

#ifndef ECMASCRIPT_RUNTIME_TRAMPOLINES_H
#define ECMASCRIPT_RUNTIME_TRAMPOLINES_H
#include "ecmascript/compiler/fast_stub_define.h"
#include "ecmascript/js_thread.h"

namespace panda::ecmascript {
class RuntimeTrampolines {
public:
    enum RuntimeTrampolineId {
#define DEF_RUNTIME_STUB(name, counter) RUNTIME_ID_##name,
        EXTERNAL_RUNTIMESTUB_LIST(DEF_RUNTIME_STUB)
#undef DEF_RUNTIME_STUB
        EXTERNAL_RUNTIME_STUB_MAXID
    };
    static void InitializeRuntimeTrampolines(JSThread *thread)
    {
    #define DEF_RUNTIME_STUB(name, counter) RuntimeTrampolineId::RUNTIME_ID_##name
    #define INITIAL_RUNTIME_FUNCTIONS(name, count) \
        thread->SetRuntimeFunction(DEF_RUNTIME_STUB(name, count), reinterpret_cast<uintptr_t>(name));
        EXTERNAL_RUNTIMESTUB_LIST(INITIAL_RUNTIME_FUNCTIONS)
    #undef INITIAL_RUNTIME_FUNCTIONS
    #undef DEF_RUNTIME_STUB
    }
    static bool AddElementInternal(uint64_t argThread, uint64_t argReceiver, uint32_t argIndex,
                                   uint64_t argValue, uint32_t argAttr);
    static bool CallSetter(uint64_t argThread, uint64_t argSetter, uint64_t argReceiver, uint64_t argValue,
                           bool argMayThrow);
    static uint64_t CallGetter(uint64_t argThread, uint64_t argGetter, uint64_t argReceiver);
    static uint64_t AccessorGetter(uint64_t argThread, uint64_t argGetter, uint64_t argReceiver);
    static void ThrowTypeError(uint64_t argThread, int argMessageStringId);
    static bool JSProxySetProperty(uint64_t argThread, uint64_t argProxy, uint64_t argKey, uint64_t argValue,
                                   uint64_t argReceiver, bool argMayThrow);
    static uint32_t GetHash32(uint64_t key, uint64_t len);
    static int32_t FindElementWithCache(uint64_t argThread, uint64_t hClass, uint64_t key, int32_t num);
    static uint32_t StringGetHashCode(uint64_t ecmaString);
    static uint64_t Execute(uint64_t argThread, uint64_t argFunc, uint64_t thisArg, uint32_t argc, uint64_t argArgv);
};

class CallRuntimeTrampolinesScope {
public:
    CallRuntimeTrampolinesScope(JSThread *thread, uintptr_t *newFp, uintptr_t *fp)
        :oldFramePointer_(nullptr),
        thread_(thread)
    {
        lastOptCallRuntimePc_ = thread->GetRuntimeTrampolinesFP();
        thread->SetRuntimeTrampolinesFP(fp);
        JSTaggedType *cursp = const_cast<JSTaggedType *>(thread->GetCurrentSPFrame());
        oldFramePointer_ = static_cast<uintptr_t *>(static_cast<void *>(cursp));
        JSTaggedType *newSp = static_cast<JSTaggedType *>(static_cast<void *>(newFp));
        thread_->SetCurrentSPFrame(newSp);
        // print newfp and type for debug
        std::cout << "CallRuntimeTrampolinesScope newFp: " << newFp << " oldFramePointer_ : " << oldFramePointer_
            << std::endl;
        FrameType type = *(reinterpret_cast<FrameType*>(
                    reinterpret_cast<long long>(newFp) + FrameConst::kFrameType));
        std::cout << __FUNCTION__ << " type = " << as_integer(type) << std::endl;
    }
    ~CallRuntimeTrampolinesScope()
    {
        // print oldfp and type for debug
        std::cout << "~CallRuntimeTrampolinesScope oldFramePointer_: " << oldFramePointer_ <<
            " thread_->fp:" << thread_->GetCurrentSPFrame() << std::endl;
        FrameType type = *(reinterpret_cast<FrameType*>(
                    reinterpret_cast<long long>(oldFramePointer_) + FrameConst::kFrameType));
        std::cout << __FUNCTION__ << "type = " << as_integer(type) << std::endl;
        JSTaggedType *oldSp = static_cast<JSTaggedType *>(static_cast<void *>(oldFramePointer_));
        thread_->SetCurrentSPFrame(oldSp);
        thread_->SetRuntimeTrampolinesFP(lastOptCallRuntimePc_);
    }
private:
    uintptr_t *oldFramePointer_;
    JSThread *thread_;
    uintptr_t  *lastOptCallRuntimePc_;
};
}  // namespace panda::ecmascript
#endif