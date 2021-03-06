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

#include "ecmascript/compiler/circuit_builder.h"
#include "include/coretypes/tagged_value.h"
#include "utils/bit_utils.h"

namespace kungfu {
AddrShift CircuitBuilder::NewArguments(size_t index)
{
    auto argListOfCircuit = Circuit::GetCircuitRoot(OpCode(OpCode::ARG_LIST));
    return circuit_->NewGate(OpCode(OpCode::JS_ARG), index, {argListOfCircuit}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewMerge(AddrShift *inList, size_t controlCount)
{
    return circuit_->NewGate(OpCode(OpCode::MERGE), controlCount, controlCount, inList, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewSelectorGate(OpCode opCode, AddrShift control, int valueCounts)
{
    std::vector<AddrShift> inList;
    inList.push_back(control);
    for (int i = 0; i < valueCounts; i++) {
        inList.push_back(Circuit::NullGate());
    }

    return circuit_->NewGate(opCode, valueCounts, inList, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewSelectorGate(OpCode opCode, AddrShift control, std::vector<AddrShift> &values,
                                          int valueCounts)
{
    std::vector<AddrShift> inList;
    inList.push_back(control);
    for (int i = 0; i < valueCounts; i++) {
        inList.push_back(values[i]);
    }

    return circuit_->NewGate(opCode, valueCounts, inList, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewIntegerConstant(int32_t val)
{
    auto constantList = Circuit::GetCircuitRoot(OpCode(OpCode::CONSTANT_LIST));
    return circuit_->NewGate(OpCode(OpCode::INT32_CONSTANT), val, {constantList}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewInteger64Constant(int64_t val)
{
    auto constantList = Circuit::GetCircuitRoot(OpCode(OpCode::CONSTANT_LIST));
    return circuit_->NewGate(OpCode(OpCode::INT64_CONSTANT), val, {constantList}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewBooleanConstant(bool val)
{
    auto constantList = Circuit::GetCircuitRoot(OpCode(OpCode::CONSTANT_LIST));
    return circuit_->NewGate(OpCode(OpCode::INT32_CONSTANT), val ? 1 : 0, {constantList}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewDoubleConstant(double val)
{
    auto constantList = Circuit::GetCircuitRoot(OpCode(OpCode::CONSTANT_LIST));
    return circuit_->NewGate(OpCode(OpCode::FLOAT64_CONSTANT), bit_cast<int64_t>(val), {constantList},
                             TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::UndefineConstant()
{
    auto constantList = Circuit::GetCircuitRoot(OpCode(OpCode::CONSTANT_LIST));
    return circuit_->NewGate(OpCode(OpCode::INT64_CONSTANT), panda::coretypes::TaggedValue::VALUE_UNDEFINED,
                             {constantList}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::HoleConstant()
{
    auto constantList = Circuit::GetCircuitRoot(OpCode(OpCode::CONSTANT_LIST));
    // NOTE: add bitfield value here
    return circuit_->NewGate(OpCode(OpCode::JS_CONSTANT), panda::coretypes::TaggedValue::VALUE_HOLE, {constantList},
                             TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::ExceptionConstant()
{
    auto constantList = Circuit::GetCircuitRoot(OpCode(OpCode::CONSTANT_LIST));
    // NOTE: add bitfield value here
    return circuit_->NewGate(OpCode(OpCode::JS_CONSTANT), panda::coretypes::TaggedValue::VALUE_EXCEPTION,
                             {constantList}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::Branch(AddrShift state, AddrShift condition)
{
    return circuit_->NewGate(OpCode(OpCode::IF_BRANCH), 0, {state, condition}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::SwitchBranch(AddrShift state, AddrShift index, int caseCounts)
{
    return circuit_->NewGate(OpCode(OpCode::SWITCH_BRANCH), caseCounts, {state, index}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::Return(AddrShift state, AddrShift depend, AddrShift value)
{
    auto returnList = Circuit::GetCircuitRoot(OpCode(OpCode::RETURN_LIST));
    return circuit_->NewGate(OpCode(OpCode::RETURN), 0, {state, depend, value, returnList}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::Goto(AddrShift state)
{
    return circuit_->NewGate(OpCode(OpCode::ORDINARY_BLOCK), 0, {state}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::LoopBegin(AddrShift state)
{
    auto nullGate = Circuit::NullGate();
    return circuit_->NewGate(OpCode(OpCode::LOOP_BEGIN), 0, {state, nullGate}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::LoopEnd(AddrShift state)
{
    return circuit_->NewGate(OpCode(OpCode::LOOP_BACK), 0, {state}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewIfTrue(AddrShift ifBranch)
{
    return circuit_->NewGate(OpCode(OpCode::IF_TRUE), 0, {ifBranch}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewIfFalse(AddrShift ifBranch)
{
    return circuit_->NewGate(OpCode(OpCode::IF_FALSE), 0, {ifBranch}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewSwitchCase(AddrShift switchBranch, int32_t value)
{
    return circuit_->NewGate(OpCode(OpCode::SWITCH_CASE), value, {switchBranch}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewDefaultCase(AddrShift switchBranch)
{
    return circuit_->NewGate(OpCode(OpCode::DEFAULT_CASE), 0, {switchBranch}, TypeCode::NOTYPE);
}

OpCode CircuitBuilder::GetStoreOpCodeFromMachineType(MachineType type)
{
    switch (type) {
        case MachineType::INT8_TYPE:
            return OpCode(OpCode::INT8_STORE);
        case MachineType::INT16_TYPE:
            return OpCode(OpCode::INT16_STORE);
        case MachineType::INT32_TYPE:
            return OpCode(OpCode::INT32_STORE);
        case MachineType::INT64_TYPE:
            return OpCode(OpCode::INT64_STORE);
        case MachineType::BOOL_TYPE:
            return OpCode(OpCode::INT32_STORE);
        case MachineType::UINT8_TYPE:
            return OpCode(OpCode::INT8_STORE);
        case MachineType::UINT16_TYPE:
            return OpCode(OpCode::INT16_STORE);
        case MachineType::UINT32_TYPE:
            return OpCode(OpCode::INT32_STORE);
        case MachineType::UINT64_TYPE:
        case MachineType::POINTER_TYPE:
        case MachineType::TAGGED_TYPE:
        case MachineType::TAGGED_POINTER_TYPE:
            return OpCode(OpCode::INT64_STORE);
        case MachineType::FLOAT32_TYPE:
            return OpCode(OpCode::FLOAT32_STORE);
        case MachineType::FLOAT64_TYPE:
            return OpCode(OpCode::FLOAT64_STORE);
        default:
            UNREACHABLE();
    }
}

OpCode CircuitBuilder::GetLoadOpCodeFromMachineType(MachineType type)
{
    switch (type) {
        case MachineType::INT8_TYPE:
            return OpCode(OpCode::INT8_LOAD);
        case MachineType::INT16_TYPE:
            return OpCode(OpCode::INT16_LOAD);
        case MachineType::INT32_TYPE:
            return OpCode(OpCode::INT32_LOAD);
        case MachineType::INT64_TYPE:
            return OpCode(OpCode::INT64_LOAD);
        case MachineType::BOOL_TYPE:
            return OpCode(OpCode::INT32_LOAD);
        case MachineType::UINT8_TYPE:
            return OpCode(OpCode::INT8_LOAD);
        case MachineType::UINT16_TYPE:
            return OpCode(OpCode::INT16_LOAD);
        case MachineType::UINT32_TYPE:
            return OpCode(OpCode::INT32_LOAD);
        case MachineType::UINT64_TYPE:
        case MachineType::POINTER_TYPE:
        case MachineType::TAGGED_TYPE:
        case MachineType::TAGGED_POINTER_TYPE:
            return OpCode(OpCode::INT64_LOAD);
        case MachineType::FLOAT32_TYPE:
            return OpCode(OpCode::FLOAT32_LOAD);
        case MachineType::FLOAT64_TYPE:
            return OpCode(OpCode::FLOAT64_LOAD);
        default:
            UNREACHABLE();
    }
}

OpCode CircuitBuilder::GetSelectOpCodeFromMachineType(MachineType type)
{
    switch (type) {
        case MachineType::NONE_TYPE:
            return OpCode(OpCode::DEPEND_SELECTOR);
        case MachineType::INT8_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT8);
        case MachineType::INT16_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT16);
        case MachineType::INT32_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT32);
        case MachineType::INT64_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT64);
        case MachineType::BOOL_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT1);
        case MachineType::UINT8_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT8);
        case MachineType::UINT16_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT16);
        case MachineType::UINT32_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT32);
        case MachineType::UINT64_TYPE:
        case MachineType::POINTER_TYPE:
        case MachineType::TAGGED_TYPE:
        case MachineType::TAGGED_POINTER_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_INT64);
        case MachineType::FLOAT32_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_FLOAT32);
        case MachineType::FLOAT64_TYPE:
            return OpCode(OpCode::VALUE_SELECTOR_FLOAT64);
        default:
            UNREACHABLE();
    }
}

AddrShift CircuitBuilder::NewDependRelay(AddrShift state, AddrShift depend)
{
    return circuit_->NewGate(OpCode(OpCode::DEPEND_RELAY), 0, {state, depend}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewDependAnd(std::initializer_list<AddrShift> args)
{
    std::vector<AddrShift> inputs;
    for (auto arg : args) {
        inputs.push_back(arg);
    }
    return circuit_->NewGate(OpCode(OpCode::DEPEND_AND), args.size(), inputs, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewLoadGate(MachineType type, AddrShift val, AddrShift depend)
{
    OpCode op = GetLoadOpCodeFromMachineType(type);
    return circuit_->NewGate(op, static_cast<BitField>(type), {depend, val}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewStoreGate(MachineType type, AddrShift ptr, AddrShift val, AddrShift depend)
{
    OpCode op = GetStoreOpCodeFromMachineType(type);
    return circuit_->NewGate(op, static_cast<BitField>(type), {depend, val, ptr}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewArithMeticGate(OpCode opcode, AddrShift left, AddrShift right)
{
    return circuit_->NewGate(opcode, 0, {left, right}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewArithMeticGate(OpCode opcode, AddrShift value)
{
    return circuit_->NewGate(opcode, 0, {value}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewLogicGate(OpCode opcode, AddrShift left, AddrShift right)
{
    return circuit_->NewGate(opcode, static_cast<BitField>(MachineType::BOOL_TYPE), {left, right}, TypeCode::NOTYPE);
}

AddrShift CircuitBuilder::NewLogicGate(OpCode opcode, AddrShift value)
{
    return circuit_->NewGate(opcode, static_cast<BitField>(MachineType::BOOL_TYPE), {value}, TypeCode::NOTYPE);
}

OpCode CircuitBuilder::GetCallOpCodeFromMachineType(MachineType type)
{
    switch (type) {
        case MachineType::NONE_TYPE:
            return OpCode(OpCode::CALL);
        case MachineType::INT8_TYPE:
            return OpCode(OpCode::INT8_CALL);
        case MachineType::INT16_TYPE:
            return OpCode(OpCode::INT16_CALL);
        case MachineType::INT32_TYPE:
            return OpCode(OpCode::INT32_CALL);
        case MachineType::INT64_TYPE:
            return OpCode(OpCode::INT64_CALL);
        case MachineType::BOOL_TYPE:
            return OpCode(OpCode::INT1_CALL);
        case MachineType::UINT8_TYPE:
            return OpCode(OpCode::INT8_CALL);
        case MachineType::UINT16_TYPE:
            return OpCode(OpCode::INT16_CALL);
        case MachineType::UINT32_TYPE:
            return OpCode(OpCode::INT32_CALL);
        case MachineType::UINT64_TYPE:
        case MachineType::POINTER_TYPE:
        case MachineType::TAGGED_TYPE:
        case MachineType::TAGGED_POINTER_TYPE:
            return OpCode(OpCode::INT64_CALL);
        case MachineType::FLOAT32_TYPE:
            return OpCode(OpCode::FLOAT32_CALL);
        case MachineType::FLOAT64_TYPE:
            return OpCode(OpCode::FLOAT64_CALL);
        default:
            UNREACHABLE();
    }
}

AddrShift CircuitBuilder::NewCallGate(StubDescriptor *descriptor, AddrShift thread, AddrShift target,
                                      std::initializer_list<AddrShift> args)
{
    std::vector<AddrShift> inputs;
    // 2 means extra two input gates (target thread)
    const size_t extraparamCnt = 2;
    auto dependEntry = Circuit::GetCircuitRoot(OpCode(OpCode::DEPEND_ENTRY));
    inputs.push_back(dependEntry);
    inputs.push_back(target);
    inputs.push_back(thread);
    for (auto arg : args) {
        inputs.push_back(arg);
    }
    OpCode opcode = GetCallOpCodeFromMachineType(descriptor->GetReturnType());
    if (descriptor->GetReturnType() == MachineType::TAGGED_POINTER_TYPE) {
        return circuit_->NewGate(opcode, args.size() + extraparamCnt, inputs, TypeCode::TAGGED_POINTER_TYPE);
    }
    return circuit_->NewGate(opcode, args.size() + extraparamCnt, inputs, TypeCode::JS_ANY);
}

AddrShift CircuitBuilder::NewCallGate(StubDescriptor *descriptor, AddrShift thread, AddrShift target,
                                      AddrShift depend, std::initializer_list<AddrShift> args)
{
    std::vector<AddrShift> inputs;
    inputs.push_back(depend);
    inputs.push_back(target);
    inputs.push_back(thread);
    for (auto arg : args) {
        inputs.push_back(arg);
    }
    OpCode opcode = GetCallOpCodeFromMachineType(descriptor->GetReturnType());
    // 2 : 2 means extra two input gates (target thread )
    return circuit_->NewGate(opcode, args.size() + 2, inputs, TypeCode::JS_ANY);
}

AddrShift CircuitBuilder::Alloca(int size)
{
    auto allocaList = Circuit::GetCircuitRoot(OpCode(OpCode::ALLOCA_LIST));
    return circuit_->NewGate(OpCode(OpCode::ALLOCA), size, {allocaList}, TypeCode::NOTYPE);
}
}  // namespace kungfu
