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

#include "ecmascript/mem/concurrent_sweeper.h"

#include "ecmascript/js_hclass-inl.h"
#include "ecmascript/mem/allocator-inl.h"
#include "ecmascript/mem/ecma_heap_manager.h"
#include "ecmascript/mem/free_object_list.h"
#include "ecmascript/mem/heap.h"
#include "ecmascript/platform/platform.h"

namespace panda::ecmascript {
ConcurrentSweeper::ConcurrentSweeper(Heap *heap, bool concurrentSweep)
    : heap_(heap), concurrentSweep_(concurrentSweep)
{
}

void ConcurrentSweeper::SweepPhases(bool compressGC)
{
    if (concurrentSweep_) {
        // Add all region to region list. Ensure all task finish
        trace::ScopedTrace scoped_trace("ConcurrentSweeper::SweepPhases");
        if (!compressGC) {
            heap_->GetOldSpace()->EnumerateRegions([this](Region *current) { AddRegion(OLD_SPACE, current); });
        }
        heap_->GetNonMovableSpace()->EnumerateRegions([this](Region *current) { AddRegion(NON_MOVABLE, current); });
        heap_->GetMachineCodeSpace()->EnumerateRegions([this](Region *current) {
            AddRegion(MACHINE_CODE_SPACE, current);
        });

        // Prepare
        isSweeping_ = true;
        startSpaceType_ = compressGC ? NON_MOVABLE : OLD_SPACE;
        for (int type = startSpaceType_; type < FREE_LIST_NUM; type++) {
            auto spaceType = static_cast<MemSpaceType>(type);
            FreeListAllocator &allocator = heap_->GetHeapManager()->GetFreeListAllocator(spaceType);
            remainderTaskNum_[type] = FREE_LIST_NUM - startSpaceType_;
            allocator.SetSweeping(true);
            allocator.RebuildFreeList();
        }

        if (!compressGC) {
            Platform::GetCurrentPlatform()->PostTask(std::make_unique<SweeperTask>(this, OLD_SPACE));
        }
        Platform::GetCurrentPlatform()->PostTask(std::make_unique<SweeperTask>(this, NON_MOVABLE));
        Platform::GetCurrentPlatform()->PostTask(std::make_unique<SweeperTask>(this, MACHINE_CODE_SPACE));
    } else {
        if (!compressGC) {
            SweepSpace(const_cast<OldSpace *>(heap_->GetOldSpace()), heap_->GetHeapManager()->GetOldSpaceAllocator());
        }
        SweepSpace(const_cast<NonMovableSpace *>(heap_->GetNonMovableSpace()),
                   heap_->GetHeapManager()->GetNonMovableSpaceAllocator());
        SweepSpace(const_cast<MachineCodeSpace *>(heap_->GetMachineCodeSpace()),
                   heap_->GetHeapManager()->GetMachineCodeSpaceAllocator());
    }
    SweepHugeSpace();
}

void ConcurrentSweeper::SweepSpace(MemSpaceType type, bool isMain)
{
    trace::ScopedTrace scoped_trace("Sweeper::SweepSpace");
    FreeListAllocator &allocator = heap_->GetHeapManager()->GetFreeListAllocator(type);
    Region *current = GetRegionSafe(type);
    while (current != nullptr) {
        FreeRegion(current, allocator, isMain);
        // Main thread sweeping region is added;
        if (!isMain) {
            AddSweptRegionSafe(type, current);
        }
        current = GetRegionSafe(type);
    }

    if (!isMain) {
        os::memory::LockHolder holder(mutexs_[type]);
        remainderTaskNum_[type]--;
        if (remainderTaskNum_[type] == 0) {
            cvs_[type].SignalAll();
        }
    }
}

void ConcurrentSweeper::SweepSpace(Space *space, FreeListAllocator &allocator)
{
    allocator.RebuildFreeList();
    space->EnumerateRegions([this, &allocator](Region *current) { FreeRegion(current, allocator); });
}

void ConcurrentSweeper::SweepHugeSpace()
{
    trace::ScopedTrace scoped_trace("SweepSpace HugeObject");
    HugeObjectSpace *space = const_cast<HugeObjectSpace *>(heap_->GetHugeObjectSpace());
    Region *currentRegion = space->GetRegionList().GetFirst();

    while (currentRegion != nullptr) {
        Region *next = currentRegion->GetNext();
        auto markBitmap = currentRegion->GetMarkBitmap();
        bool isMarked = false;
        markBitmap->IterateOverMarkedChunks([&isMarked]([[maybe_unused]] void *mem) { isMarked = true; });
        if (!isMarked) {
            space->GetRegionList().RemoveNode(currentRegion);
            space->ClearAndFreeRegion(currentRegion);
        }
        currentRegion = next;
    }
}

void ConcurrentSweeper::FreeRegion(Region *current, FreeListAllocator &allocator, bool isMain)
{
    auto markBitmap = current->GetMarkBitmap();
    ASSERT(markBitmap != nullptr);
    uintptr_t freeStart = current->GetBegin();
    markBitmap->IterateOverMarkedChunks([this, &current, &freeStart, &allocator, isMain](void *mem) {
        ASSERT(current->InRange(ToUintPtr(mem)));
        auto header = reinterpret_cast<TaggedObject *>(mem);
        auto klass = header->GetClass();
        JSType jsType = klass->GetObjectType();
        auto size = klass->SizeFromJSHClass(jsType, header);
        size = AlignUp(size, static_cast<size_t>(MemAlignment::MEM_ALIGN_OBJECT));

        uintptr_t freeEnd = ToUintPtr(mem);
        if (freeStart != freeEnd) {
            FreeLiveRange(allocator, current, freeStart, freeEnd, isMain);
        }
        freeStart = freeEnd + size;
    });
    uintptr_t freeEnd = current->GetEnd();
    if (freeStart != freeEnd) {
        FreeLiveRange(allocator, current, freeStart, freeEnd, isMain);
    }
}

void ConcurrentSweeper::FillSweptRegion(MemSpaceType type)
{
    trace::ScopedTrace scoped_trace("Sweeper::FillSweptRegion");
    if (sweptList_[type].empty()) {
        return;
    }
    FreeListAllocator &allocator = heap_->GetHeapManager()->GetFreeListAllocator(type);
    Region *region = nullptr;
    while ((region = GetSweptRegionSafe(type)) != nullptr) {
        region->EnumerateKinds([&allocator](FreeObjectKind *kind) {
            if (kind == nullptr || kind->Empty()) {
                return;
            }
            allocator.FillFreeList(kind);
        });
    }
}

void ConcurrentSweeper::FreeLiveRange(FreeListAllocator &allocator, Region *current, uintptr_t freeStart,
                                      uintptr_t freeEnd, bool isMain)
{
    allocator.Free(freeStart, freeEnd, isMain);
    heap_->ClearSlotsRange(current, freeStart, freeEnd);
}

void ConcurrentSweeper::AddRegion(MemSpaceType type, Region *region)
{
    sweepingList_[type].emplace_back(region);
}

Region *ConcurrentSweeper::GetRegionSafe(MemSpaceType type)
{
    os::memory::LockHolder holder(mutexs_[type]);
    Region *region = nullptr;
    if (!sweepingList_[type].empty()) {
        region = sweepingList_[type].back();
        sweepingList_[type].pop_back();
    }
    return region;
}

void ConcurrentSweeper::AddSweptRegionSafe(MemSpaceType type, Region *region)
{
    os::memory::LockHolder holder(mutexs_[type]);
    sweptList_[type].emplace_back(region);
}

Region *ConcurrentSweeper::GetSweptRegionSafe(MemSpaceType type)
{
    os::memory::LockHolder holder(mutexs_[type]);
    Region *region = nullptr;
    if (!sweptList_[type].empty()) {
        region = sweptList_[type].back();
        sweptList_[type].pop_back();
    }
    return region;
}

void ConcurrentSweeper::EnsureAllTaskFinish()
{
    if (!isSweeping_) {
        return;
    }
    for (int i = startSpaceType_; i < FREE_LIST_NUM; i++) {
        WaitingTaskFinish(static_cast<MemSpaceType>(i));
    }
    isSweeping_ = false;
}

void ConcurrentSweeper::WaitingTaskFinish(MemSpaceType type)
{
    if (remainderTaskNum_[type] > 0) {
        SweepSpace(type);
        {
            os::memory::LockHolder holder(mutexs_[type]);
            while (remainderTaskNum_[type] > 0) {
                cvs_[type].Wait(&mutexs_[type]);
            }
        }
    }
    FinishSweeping(type);
}

void ConcurrentSweeper::FinishSweeping(MemSpaceType type)
{
    FillSweptRegion(type);
    FreeListAllocator &allocator = heap_->GetHeapManager()->GetFreeListAllocator(type);
    allocator.SetSweeping(false);
    if (type == OLD_SPACE) {
        heap_->RecomputeLimits();
    }
}

bool ConcurrentSweeper::SweeperTask::Run()
{
    int sweepTypeNum = FREE_LIST_NUM - sweeper_->startSpaceType_;
    for (size_t i = sweeper_->startSpaceType_; i < FREE_LIST_NUM; i++) {
        auto type = static_cast<MemSpaceType>(((i + type_) % sweepTypeNum) + sweeper_->startSpaceType_);
        sweeper_->SweepSpace(type, false);
    }
    return true;
}
}  // namespace panda::ecmascript
