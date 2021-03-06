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

#ifndef ECMASCRIPT_COMPILER_Stub_H
#define ECMASCRIPT_COMPILER_Stub_H

#include "ecmascript/accessor_data.h"
#include "ecmascript/base/number_helper.h"
#include "ecmascript/compiler/circuit.h"
#include "ecmascript/compiler/circuit_builder.h"
#include "ecmascript/compiler/gate.h"
#include "ecmascript/compiler/machine_type.h"
#include "ecmascript/compiler/stub_descriptor.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_object.h"
#include "ecmascript/js_tagged_value.h"
#include "ecmascript/layout_info.h"
#include "ecmascript/message_string.h"
#include "ecmascript/tagged_dictionary.h"

namespace kungfu {
using namespace panda::ecmascript;

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define DEFVARIABLE(varname, type, val) Stub::Variable varname(GetEnvironment(), type, NextVariableId(), val)

class Stub {
public:
    class Environment;
    class Label;
    class Variable;

    class Label {
    public:
        class LabelImpl {
        public:
            LabelImpl(Environment *env, AddrShift control)
                : env_(env), control_(control), predeControl_(-1), isSealed_(false)
            {
            }

            ~LabelImpl() = default;

            NO_MOVE_SEMANTIC(LabelImpl);
            NO_COPY_SEMANTIC(LabelImpl);
            void Seal();
            void WriteVariable(Variable *var, AddrShift value);
            AddrShift ReadVariable(Variable *var);
            void Bind();
            void MergeAllControl();
            void MergeAllDepend();
            void AppendPredecessor(LabelImpl *predecessor);
            std::vector<LabelImpl *> GetPredecessors() const
            {
                return predecessors_;
            }

            void SetControl(AddrShift control)
            {
                control_ = control;
            }

            void SetPreControl(AddrShift control)
            {
                predeControl_ = control;
            }

            void MergeControl(AddrShift control)
            {
                if (predeControl_ == -1) {
                    predeControl_ = control;
                    control_ = predeControl_;
                } else {
                    otherPredeControls_.push_back(control);
                }
            }

            AddrShift GetControl() const
            {
                return control_;
            }

            void SetDepend(AddrShift depend)
            {
                depend_ = depend;
            }

            AddrShift GetDepend() const
            {
                return depend_;
            }

        private:
            bool IsNeedSeal() const;
            bool IsSealed() const
            {
                return isSealed_;
            }
            bool IsLoopHead() const;
            bool IsControlCase() const;
            AddrShift ReadVariableRecursive(Variable *var);
            Environment *env_;
            AddrShift control_;
            AddrShift predeControl_ {-1};
            AddrShift dependRelay_ {-1};
            AddrShift depend_ {-1};
            AddrShift loopDepend_{-1};
            std::vector<AddrShift> otherPredeControls_;
            bool isSealed_ {false};
            std::map<Variable *, AddrShift> valueMap_;
            std::vector<AddrShift> phi;
            std::vector<LabelImpl *> predecessors_;
            std::map<Variable *, AddrShift> incompletePhis_;
        };
        explicit Label() = default;
        explicit Label(Environment *env);
        explicit Label(LabelImpl *impl) : impl_(impl) {}
        ~Label() = default;
        Label(Label const &label) = default;
        Label &operator=(Label const &label) = default;
        Label(Label &&label) = default;
        Label &operator=(Label &&label) = default;

        void Seal()
        {
            return impl_->Seal();
        }

        void WriteVariable(Variable *var, AddrShift value)
        {
            impl_->WriteVariable(var, value);
        }

        AddrShift ReadVariable(Variable *var)
        {
            return impl_->ReadVariable(var);
        }

        void Bind()
        {
            impl_->Bind();
        }

        void MergeAllControl()
        {
            impl_->MergeAllControl();
        }

        void MergeAllDepend()
        {
            impl_->MergeAllDepend();
        }

        void AppendPredecessor(const Label *predecessor)
        {
            impl_->AppendPredecessor(predecessor->GetRawLabel());
        }

        std::vector<Label> GetPredecessors() const
        {
            std::vector<Label> labels;
            for (auto rawlabel : impl_->GetPredecessors()) {
                labels.emplace_back(Label(rawlabel));
            }
            return labels;
        }

        void SetControl(AddrShift control)
        {
            impl_->SetControl(control);
        }

        void SetPreControl(AddrShift control)
        {
            impl_->SetPreControl(control);
        }

        void MergeControl(AddrShift control)
        {
            impl_->MergeControl(control);
        }

        AddrShift GetControl() const
        {
            return impl_->GetControl();
        }

        AddrShift GetDepend() const
        {
            return impl_->GetDepend();
        }

        void SetDepend(AddrShift depend)
        {
            return impl_->SetDepend(depend);
        }
    private:
        friend class Environment;
        LabelImpl *GetRawLabel() const
        {
            return impl_;
        }
        LabelImpl *impl_ {nullptr};
    };

    class Environment {
    public:
        using LabelImpl = Label::LabelImpl;
        explicit Environment(size_t arguments, Circuit *circuit);
        ~Environment();
        NO_COPY_SEMANTIC(Environment);
        NO_MOVE_SEMANTIC(Environment);
        Label *GetCurrentLabel() const
        {
            return currentLabel_;
        }
        void SetCurrentLabel(Label *label)
        {
            currentLabel_ = label;
        }
        CircuitBuilder &GetCircuitBuilder()
        {
            return builder_;
        }
        AddrShift GetArgument(size_t index) const
        {
            return arguments_.at(index);
        }
        Circuit *GetCircuit()
        {
            return circuit_;
        }

        Label GetLabelFromSelector(AddrShift sel)
        {
            LabelImpl *rawlabel = phi_to_labels_[sel];
            return Label(rawlabel);
        }

        void AddSelectorToLabel(AddrShift sel, Label label)
        {
            phi_to_labels_[sel] = label.GetRawLabel();
        }

        LabelImpl *NewLabel(Environment *env, AddrShift control = -1)
        {
            auto impl = new LabelImpl(env, control);
            rawlabels_.emplace_back(impl);
            return impl;
        }

        void PushCurrentLabel(Label *entry)
        {
            AddrShift control = currentLabel_->GetControl();
            AddrShift depend = currentLabel_->GetDepend();
            if (currentLabel_ != nullptr) {
                stack_.push(currentLabel_);
                currentLabel_ = entry;
                currentLabel_->SetControl(control);
                currentLabel_->SetDepend(depend);
            }
        }

        void PopCurrentLabel()
        {
            AddrShift control = currentLabel_->GetControl();
            AddrShift depend = currentLabel_->GetDepend();
            if (!stack_.empty()) {
                currentLabel_ = stack_.top();
                currentLabel_->SetControl(control);
                currentLabel_->SetDepend(depend);
                stack_.pop();
            }
        }

        void SetFrameType(panda::ecmascript::FrameType type)
        {
            circuit_->SetFrameType(type);
        }

    private:
        Label *currentLabel_ {nullptr};
        Circuit *circuit_;
        CircuitBuilder builder_;
        std::unordered_map<AddrShift, LabelImpl *> phi_to_labels_;
        std::vector<AddrShift> arguments_;
        Label entry_;
        std::vector<LabelImpl *> rawlabels_;
        std::stack<Label *> stack_;
    };

    class Variable {
    public:
        Variable(Environment *env, MachineType type, uint32_t id, AddrShift value) : id_(id), type_(type), env_(env)
        {
            Bind(value);
            env_->GetCurrentLabel()->WriteVariable(this, value);
        }
        ~Variable() = default;
        NO_MOVE_SEMANTIC(Variable);
        NO_COPY_SEMANTIC(Variable);
        void Bind(AddrShift value)
        {
            currentValue_ = value;
        }
        AddrShift Value() const
        {
            return currentValue_;
        }
        MachineType Type() const
        {
            return type_;
        }
        bool IsBound() const
        {
            return currentValue_ != 0;
        }
        Variable &operator=(const AddrShift value)
        {
            env_->GetCurrentLabel()->WriteVariable(this, value);
            Bind(value);
            return *this;
        }

        AddrShift operator*()
        {
            return env_->GetCurrentLabel()->ReadVariable(this);
        }

        AddrShift AddPhiOperand(AddrShift val);
        AddrShift AddOperandToSelector(AddrShift val, size_t idx, AddrShift in);
        AddrShift TryRemoveTrivialPhi(AddrShift phi);
        void RerouteOuts(const std::vector<Out *> &outs, Gate *newGate);

        bool IsSelector(AddrShift gate) const
        {
            return env_->GetCircuit()->IsSelector(gate);
        }

        bool IsSelector(const Gate *gate) const
        {
            return gate->GetOpCode() >= OpCode::VALUE_SELECTOR_JS
                   && gate->GetOpCode() <= OpCode::VALUE_SELECTOR_FLOAT64;
        }

        uint32_t GetId() const
        {
            return id_;
        }

    private:
        uint32_t id_;
        MachineType type_;
        AddrShift currentValue_ {0};
        Environment *env_;
    };

    class SubCircuitScope {
    public:
        explicit SubCircuitScope(Environment *env, Label *entry) : env_(env)
        {
            env_->PushCurrentLabel(entry);
        }
        ~SubCircuitScope()
        {
            env_->PopCurrentLabel();
        }

    private:
        Environment *env_;
    };

    explicit Stub(const char *name, int argCount, Circuit *circuit)
        : env_(argCount, circuit), methodName_(name)
    {
    }
    virtual ~Stub() = default;
    NO_MOVE_SEMANTIC(Stub);
    NO_COPY_SEMANTIC(Stub);
    virtual void GenerateCircuit() = 0;

    Environment *GetEnvironment()
    {
        return &env_;
    }
    // constant
    AddrShift GetInt32Constant(int32_t value)
    {
        return env_.GetCircuitBuilder().NewIntegerConstant(value);
    };
    AddrShift GetWord64Constant(uint64_t value)
    {
        return env_.GetCircuitBuilder().NewInteger64Constant(value);
    };

    AddrShift GetPtrConstant(uint32_t value)
    {
#ifdef PANDA_TARGET_AMD64
        return GetWord64Constant(value);
#endif
#ifdef PANDA_TARGET_X86
        return GetInt32Constant(value);
#endif
#ifdef PANDA_TARGET_ARM64
        return GetWord64Constant(value);
#endif
#ifdef PANDA_TARGET_ARM32
        return GetInt32Constant(value);
#endif
    }

    AddrShift PtrMul(AddrShift x, AddrShift y)
    {
#ifdef PANDA_TARGET_AMD64
        return Int64Mul(x, y);
#endif
#ifdef PANDA_TARGET_X86
        return Int32Mul(x, y);
#endif
#ifdef PANDA_TARGET_ARM64
        return Int64Mul(x, y);
#endif
#ifdef PANDA_TARGET_ARM32
        return Int32Mul(x, y);
#endif
    }

    AddrShift TrueConstant()
    {
        return TruncInt32ToInt1(GetInt32Constant(1));
    }

    AddrShift FalseConstant()
    {
        return TruncInt32ToInt1(GetInt32Constant(0));
    }

    AddrShift GetBooleanConstant(bool value)
    {
        return env_.GetCircuitBuilder().NewBooleanConstant(value);
    }

    AddrShift GetDoubleConstant(double value)
    {
        return env_.GetCircuitBuilder().NewDoubleConstant(value);
    }

    AddrShift GetUndefinedConstant()
    {
        return env_.GetCircuitBuilder().UndefineConstant();
    }

    AddrShift GetHoleConstant()
    {
        return env_.GetCircuitBuilder().HoleConstant();
    }

    AddrShift GetExceptionConstant()
    {
        return env_.GetCircuitBuilder().ExceptionConstant();
    }

    // parameter
    AddrShift Argument(size_t index)
    {
        return env_.GetArgument(index);
    }

    AddrShift Int1Argument(size_t index)
    {
        AddrShift argument = Argument(index);
        env_.GetCircuit()->SetOpCode(argument, OpCode(OpCode::INT1_ARG));
        return argument;
    }

    AddrShift Int32Argument(size_t index)
    {
        AddrShift argument = Argument(index);
        env_.GetCircuit()->SetOpCode(argument, OpCode(OpCode::INT32_ARG));
        return argument;
    }

    AddrShift Int64Argument(size_t index)
    {
        AddrShift argument = Argument(index);
        env_.GetCircuit()->SetOpCode(argument, OpCode(OpCode::INT64_ARG));
        return argument;
    }

    AddrShift PtrArgument(size_t index)
    {
        AddrShift argument = Argument(index);
        if (PtrValueCode() == INT64) {
            env_.GetCircuit()->SetOpCode(argument, OpCode(OpCode::INT64_ARG));
        } else if (PtrValueCode() == INT32) {
            env_.GetCircuit()->SetOpCode(argument, OpCode(OpCode::INT32_ARG));
        } else {
            UNREACHABLE();
        }
        return argument;
    }

    AddrShift Float32Argument(size_t index)
    {
        AddrShift argument = Argument(index);
        env_.GetCircuit()->SetOpCode(argument, OpCode(OpCode::FLOAT32_ARG));
        return argument;
    }

    AddrShift Float64Argument(size_t index)
    {
        AddrShift argument = Argument(index);
        env_.GetCircuit()->SetOpCode(argument, OpCode(OpCode::FLOAT64_ARG));
        return argument;
    }

    AddrShift Alloca(int size)
    {
        return env_.GetCircuitBuilder().Alloca(size);
    }

    // control flow
    AddrShift Return(AddrShift value)
    {
        auto control = env_.GetCurrentLabel()->GetControl();
        auto depend = env_.GetCurrentLabel()->GetDepend();
        return env_.GetCircuitBuilder().Return(control, depend, value);
    }

    void Bind(Label *label)
    {
        label->Bind();
        env_.SetCurrentLabel(label);
    }
    void Jump(Label *label);
    void Branch(AddrShift condition, Label *trueLabel, Label *falseLabel);
    void Switch(AddrShift index, Label *defaultLabel, int32_t *keysValue, Label *keysLabel, int numberOfKeys);
    void Seal(Label *label)
    {
        label->Seal();
    }

    void LoopBegin(Label *loopHead);
    void LoopEnd(Label *loopHead);

    // call operation
    AddrShift CallStub(StubDescriptor *descriptor,  AddrShift thread, AddrShift target,
                       std::initializer_list<AddrShift> args)
    {
        auto depend = env_.GetCurrentLabel()->GetDepend();
        AddrShift result = env_.GetCircuitBuilder().NewCallGate(descriptor, thread, target, depend, args);
        env_.GetCurrentLabel()->SetDepend(result);
        return result;
    }
    AddrShift CallStub(StubDescriptor *descriptor,  AddrShift thread, AddrShift target, AddrShift depend,
                       std::initializer_list<AddrShift> args)
    {
        AddrShift result = env_.GetCircuitBuilder().NewCallGate(descriptor, thread, target, depend, args);
        env_.GetCurrentLabel()->SetDepend(result);
        return result;
    }

    AddrShift CallRuntime(StubDescriptor *descriptor, AddrShift thread, AddrShift target,
                          std::initializer_list<AddrShift> args)
    {
        auto depend = env_.GetCurrentLabel()->GetDepend();
        AddrShift result = env_.GetCircuitBuilder().NewCallGate(descriptor, thread, target, depend, args);
        env_.GetCurrentLabel()->SetDepend(result);
        return result;
    }

    AddrShift CallRuntime(StubDescriptor *descriptor, AddrShift thread, AddrShift target, AddrShift depend,
                          std::initializer_list<AddrShift> args)
    {
        AddrShift result = env_.GetCircuitBuilder().NewCallGate(descriptor, thread, target, depend, args);
        env_.GetCurrentLabel()->SetDepend(result);
        return result;
    }

    // memory
    AddrShift Load(MachineType type, AddrShift base, AddrShift offset)
    {
        auto depend = env_.GetCurrentLabel()->GetDepend();
        if (PtrValueCode() == ValueCode::INT64) {
            AddrShift val = Int64Add(base, offset);
            AddrShift result = env_.GetCircuitBuilder().NewLoadGate(type, val, depend);
            env_.GetCurrentLabel()->SetDepend(result);
            return result;
        }
        if (PtrValueCode() == ValueCode::INT32) {
            AddrShift val = Int32Add(base, offset);
            AddrShift result = env_.GetCircuitBuilder().NewLoadGate(type, val, depend);
            env_.GetCurrentLabel()->SetDepend(result);
            return result;
        }
        UNREACHABLE();
    }

    AddrShift Load(MachineType type, AddrShift base)
    {
        auto depend = env_.GetCurrentLabel()->GetDepend();
        AddrShift result = env_.GetCircuitBuilder().NewLoadGate(type, base, depend);
        env_.GetCurrentLabel()->SetDepend(result);
        return result;
    }

    AddrShift LoadFromObject(MachineType type, AddrShift object, AddrShift offset);

    AddrShift Store(MachineType type, AddrShift thread, AddrShift base, AddrShift offset, AddrShift value);

    AddrShift Store(MachineType type, AddrShift base, AddrShift offset, AddrShift value);

    // arithmetic
    AddrShift Int32Add(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_ADD), x, y);
    }

    AddrShift Int64Add(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_ADD), x, y);
    }

    AddrShift DoubleAdd(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::FLOAT64_ADD), x, y);
    }

    AddrShift PtrAdd(AddrShift x, AddrShift y)
    {
#ifdef PANDA_TARGET_AMD64
        return Int64Add(x, y);
#endif
#ifdef PANDA_TARGET_X86
        return Int32Add(x, y);
#endif
#ifdef PANDA_TARGET_ARM64
        return Int64Add(x, y);
#endif
#ifdef PANDA_TARGET_ARM32
        return Int32Add(x, y);
#endif
    }

    AddrShift Int32Sub(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_SUB), x, y);
    }

    AddrShift Int64Sub(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_SUB), x, y);
    }

    AddrShift DoubleSub(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::FLOAT64_SUB), x, y);
    }

    AddrShift Int32Mul(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_MUL), x, y);
    }

    AddrShift Int64Mul(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_MUL), x, y);
    }

    AddrShift DoubleMul(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::FLOAT64_MUL), x, y);
    }

    AddrShift DoubleDiv(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::FLOAT64_DIV), x, y);
    }
    AddrShift Int32Div(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_SDIV), x, y);
    }

    AddrShift Word32Div(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_UDIV), x, y);
    }

    AddrShift Int64Div(AddrShift x, AddrShift y);

    AddrShift Int32Mod(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_SMOD), x, y);
    }

    AddrShift DoubleMod(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::FLOAT64_SMOD), x, y);
    }

    // bit operation
    AddrShift Word32Or(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_OR), x, y);
    }

    AddrShift Word32And(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_AND), x, y);
    }

    AddrShift Word32Not(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_REV), x);
    }

    AddrShift Word32Xor(AddrShift x, AddrShift y);

    AddrShift FixLoadType(AddrShift x);

    AddrShift Word64Or(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_OR), x, y);
    }

    AddrShift Word64And(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_AND), x, y);
    }

    AddrShift Word64Xor(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_XOR), x, y);
    }

    AddrShift Word64Not(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_REV), x);
    }

    AddrShift Word32LSL(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_LSL), x, y);
    }
    AddrShift Word64LSL(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_LSL), x, y);
    }
    AddrShift Word32LSR(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_LSR), x, y);
    }
    AddrShift Word64LSR(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT64_LSR), x, y);
    }
    AddrShift Word32Sar(AddrShift x, AddrShift y);
    AddrShift Word64Sar(AddrShift x, AddrShift y);
    AddrShift TaggedIsInt(AddrShift x)
    {
        return Word64Equal(Word64And(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::TAG_MASK)),
                           GetWord64Constant(panda::ecmascript::JSTaggedValue::TAG_INT));
    }

    AddrShift TaggedIsDouble(AddrShift x)
    {
        return Word32Equal(Word32Or(SExtInt1ToInt32(TaggedIsInt(x)), SExtInt1ToInt32(TaggedIsObject(x))),
                           GetInt32Constant(0));
    }

    AddrShift TaggedIsObject(AddrShift x)
    {
        return Word64Equal(Word64And(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::TAG_MASK)),
                           GetWord64Constant(panda::ecmascript::JSTaggedValue::TAG_OBJECT));
    }

    AddrShift TaggedIsNumber(AddrShift x)
    {
        return TruncInt32ToInt1(Word32Or(SExtInt1ToInt32(TaggedIsInt(x)), SExtInt1ToInt32(TaggedIsDouble(x))));
    }

    AddrShift TaggedIsHole(AddrShift x)
    {
        return Word64Equal(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_HOLE));
    }

    AddrShift TaggedIsNotHole(AddrShift x)
    {
        return Word64NotEqual(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_HOLE));
    }

    AddrShift TaggedIsUndefined(AddrShift x)
    {
        return Word64Equal(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_UNDEFINED));
    }

    AddrShift TaggedIsSpecial(AddrShift x)
    {
        return TruncInt32ToInt1(Word32And(
            SExtInt1ToInt32(
                Word64Equal(Word64And(x, GetWord64Constant(~panda::ecmascript::JSTaggedValue::TAG_SPECIAL_MASK)),
                            GetWord64Constant(0))),
            Word32Or(SExtInt1ToInt32(Word64NotEqual(
                Word64And(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::TAG_SPECIAL_VALUE)),
                GetWord64Constant(0))), SExtInt1ToInt32(TaggedIsHole(x)))));
    }

    AddrShift TaggedIsHeapObject(AddrShift x)
    {
        return TruncInt32ToInt1(
            Word32And(SExtInt1ToInt32(TaggedIsObject(x)),
                      SExtInt1ToInt32(Word32Equal(SExtInt1ToInt32(TaggedIsSpecial(x)), GetInt32Constant(0)))));
    }

    AddrShift TaggedIsString(AddrShift obj);

    AddrShift TaggedIsStringOrSymbol(AddrShift obj);

    AddrShift GetNextPositionForHash(AddrShift last, AddrShift count, AddrShift size)
    {
        auto nextOffset = Word32LSR(Int32Mul(count, Int32Add(count, GetInt32Constant(1))),
                                    GetInt32Constant(1));
        return Word32And(Int32Add(last, nextOffset), Int32Sub(size, GetInt32Constant(1)));
    }

    AddrShift DoubleIsNAN(AddrShift x)
    {
        AddrShift diff = DoubleEqual(x, x);
        return Word32Equal(SExtInt1ToInt32(diff), GetInt32Constant(0));
    }

    AddrShift DoubleIsINF(AddrShift x)
    {
        AddrShift infinity = GetDoubleConstant(base::POSITIVE_INFINITY);
        AddrShift negativeInfinity = GetDoubleConstant(-base::POSITIVE_INFINITY);
        AddrShift diff1 = DoubleEqual(x, infinity);
        AddrShift diff2 = DoubleEqual(x, negativeInfinity);
        return TruncInt32ToInt1(Word32Or(Word32Equal(SExtInt1ToInt32(diff1), GetInt32Constant(1)),
            Word32Equal(SExtInt1ToInt32(diff2), GetInt32Constant(1))));
    }

        AddrShift TaggedIsNull(AddrShift x)
    {
        return Word64Equal(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_NULL));
    }

    AddrShift TaggedIsUndefinedOrNull(AddrShift x)
    {
        return TruncInt32ToInt1(Word32Or(SExtInt1ToInt32(TaggedIsUndefined(x)), SExtInt1ToInt32(TaggedIsNull(x))));
    }

    AddrShift TaggedIsTrue(AddrShift x)
    {
        return Word64Equal(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_TRUE));
    }

    AddrShift TaggedIsFalse(AddrShift x)
    {
        return Word64Equal(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_FALSE));
    }

    AddrShift TaggedIsBoolean(AddrShift x)
    {
        return TruncInt32ToInt1(Word32Or(SExtInt1ToInt32(TaggedIsTrue(x)), SExtInt1ToInt32(TaggedIsFalse(x))));
    }

    AddrShift IntBuildTagged(AddrShift x)
    {
        AddrShift val = ZExtInt32ToInt64(x);
        return Word64Or(val, GetWord64Constant(panda::ecmascript::JSTaggedValue::TAG_INT));
    }

    AddrShift Int64BuildTagged(AddrShift x)
    {
        return Word64Or(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::TAG_INT));
    }

    AddrShift DoubleBuildTagged(AddrShift x)
    {
        AddrShift val = CastDoubleToInt64(x);
        return Int64Add(val, GetWord64Constant(panda::ecmascript::JSTaggedValue::DOUBLE_ENCODE_OFFSET));
    }

    AddrShift CastDoubleToInt64(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::BITCAST_FLOAT64_TO_INT64), x);
    }

    AddrShift TaggedTrue()
    {
        return GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_TRUE);
    }

    AddrShift TaggedFalse()
    {
        return GetWord64Constant(panda::ecmascript::JSTaggedValue::VALUE_FALSE);
    }

    // compare operation
    AddrShift Word32Equal(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_EQ), x, y);
    }

    AddrShift Word32NotEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_NE), x, y);
    }

    AddrShift Word64Equal(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_EQ), x, y);
    }

    AddrShift DoubleEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::FLOAT64_EQ), x, y);
    }

    AddrShift Word64NotEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_NE), x, y);
    }

    AddrShift Int32GreaterThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_SGT), x, y);
    }

    AddrShift Int32LessThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_SLT), x, y);
    }

    AddrShift Int32GreaterThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_SGE), x, y);
    }

    AddrShift Int32LessThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_SLE), x, y);
    }

    AddrShift Word32GreaterThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_UGT), x, y);
    }

    AddrShift Word32LessThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_ULT), x, y);
    }

    AddrShift Word32LessThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_ULE), x, y);
    }

    AddrShift Word32GreaterThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT32_UGE), x, y);
    }

    AddrShift Int64GreaterThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_SGT), x, y);
    }

    AddrShift Int64LessThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_SLT), x, y);
    }

    AddrShift Int64LessThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_SLE), x, y);
    }

    AddrShift Int64GreaterThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_SGE), x, y);
    }

    AddrShift Word64GreaterThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_UGT), x, y);
    }

    AddrShift Word64LessThan(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_ULT), x, y);
    }

    AddrShift Word64LessThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_ULE), x, y);
    }

    AddrShift Word64GreaterThanOrEqual(AddrShift x, AddrShift y)
    {
        return env_.GetCircuitBuilder().NewLogicGate(OpCode(OpCode::INT64_UGE), x, y);
    }

    // cast operation
    AddrShift ChangeInt64ToInt32(AddrShift val)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::TRUNC_INT64_TO_INT32), val);
    }

    AddrShift ChangeInt64ToPointer(AddrShift val)
    {
        if (PtrValueCode() == ValueCode::INT32) {
            return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::TRUNC_INT64_TO_INT32), val);
        }
        return val;
    }

    AddrShift ChangeInt32ToPointer(AddrShift val)
    {
        if (PtrValueCode() == ValueCode::INT32) {
            return val;
        }
        return ZExtInt32ToInt64(val);
    }

    // math operation
    AddrShift Sqrt(AddrShift x);

    AddrShift GetSetterFromAccessor(AddrShift accessor)
    {
        AddrShift setterOffset = GetPtrConstant(panda::ecmascript::AccessorData::SETTER_OFFSET);
        return Load(MachineType::UINT64_TYPE, accessor, setterOffset);
    }

    AddrShift GetElements(AddrShift object)
    {
        AddrShift elementsOffset = GetInt32Constant(panda::ecmascript::JSObject::ELEMENTS_OFFSET);
        if (PtrValueCode() == ValueCode::INT64) {
            elementsOffset = SExtInt32ToInt64(elementsOffset);
        }
        // load elements in object
        return Load(MachineType::UINT64_TYPE, object, elementsOffset);
    }

    AddrShift GetProperties(AddrShift object)
    {
        AddrShift propertiesOffset = GetInt32Constant(panda::ecmascript::JSObject::PROPERTIES_OFFSET);
        if (PtrValueCode() == ValueCode::INT64) {
            propertiesOffset = SExtInt32ToInt64(propertiesOffset);
        }
        // load properties in object
        return Load(MachineType::UINT64_TYPE, object, propertiesOffset);
    }

    // setProperties in js_object.h
    void setProperties(AddrShift thread, AddrShift object, AddrShift value)
    {
        AddrShift propertiesOffset = GetInt32Constant(panda::ecmascript::JSObject::PROPERTIES_OFFSET);
        if (PtrValueCode() == ValueCode::INT64) {
            propertiesOffset = SExtInt32ToInt64(propertiesOffset);
        }
        Store(MachineType::UINT64_TYPE, thread, object, propertiesOffset, value);
    }

    AddrShift GetLengthofElements(AddrShift elements)
    {
        return Load(MachineType::UINT32_TYPE, elements, GetPtrConstant(panda::coretypes::Array::GetLengthOffset()));
    }

    // object operation
    AddrShift LoadHClass(AddrShift object)
    {
        return ChangeInt32ToPointer(Load(MachineType::UINT32_TYPE, object));
    }

    AddrShift GetObjectType(AddrShift hClass)
    {
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);
        AddrShift bitfield = Load(MachineType::UINT64_TYPE, hClass, bitfieldOffset);
        return ChangeInt64ToInt32(
            Word64And(bitfield, GetWord64Constant((1LLU << panda::ecmascript::JSHClass::ObjectTypeBits::SIZE) - 1)));
    }

    AddrShift IsDictionaryMode(AddrShift object)
    {
        AddrShift objectType = GetObjectType(LoadHClass(object));
        return Word32Equal(objectType,
            GetInt32Constant(static_cast<int32_t>(panda::ecmascript::JSType::TAGGED_DICTIONARY)));
    }

    AddrShift IsDictionaryModeByHClass(AddrShift hClass)
    {
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);
        AddrShift bitfield = Load(MachineType::INT64_TYPE, hClass, bitfieldOffset);
        return Word64NotEqual(
            Word64And(
                Word64LSR(bitfield, GetWord64Constant(panda::ecmascript::JSHClass::IsDictionaryBit::START_BIT)),
                GetWord64Constant((1LLU << panda::ecmascript::JSHClass::IsDictionaryBit::SIZE) - 1)),
            GetWord64Constant(0));
    }

    AddrShift IsDictionaryElement(AddrShift hClass)
    {
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);
        AddrShift bitfield = Load(MachineType::INT64_TYPE, hClass, bitfieldOffset);
        // decode
        return Word64NotEqual(
            Word64And(
                Word64LSR(bitfield, GetWord64Constant(panda::ecmascript::JSHClass::DictionaryElementBits::START_BIT)),
                GetWord64Constant((1LLU << panda::ecmascript::JSHClass::DictionaryElementBits::SIZE) - 1)),
            GetWord64Constant(0));
    }

    AddrShift NotBuiltinsConstructor(AddrShift object)
    {
        AddrShift hclass = LoadHClass(object);
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);

        AddrShift bitfield = Load(MachineType::INT64_TYPE, hclass, bitfieldOffset);
        // decode
        return Word64Equal(
            Word64And(
                Word64LSR(bitfield, GetWord64Constant(panda::ecmascript::JSHClass::BuiltinsCtorBit::START_BIT)),
                GetWord64Constant((1LLU << panda::ecmascript::JSHClass::BuiltinsCtorBit::SIZE) - 1)),
            GetWord64Constant(0));
    }

    AddrShift IsClassConstructor(AddrShift object)
    {
        AddrShift functionInfoFlagOffset = GetPtrConstant(panda::ecmascript::JSFunction::FUNCTION_INFO_FLAG_OFFSET);
        AddrShift functionInfoTaggedValue = Load(MachineType::UINT64_TYPE, object, functionInfoFlagOffset);
        AddrShift functionInfoInt32 = TaggedCastToInt32(functionInfoTaggedValue);
        AddrShift functionInfoFlag = ZExtInt32ToInt64(functionInfoInt32);
        // decode
        return Word64NotEqual(
            Word64And(
                Word64LSR(functionInfoFlag,
                          GetWord64Constant(panda::ecmascript::JSFunction::ClassConstructorBit::START_BIT)),
                GetWord64Constant((1LLU << panda::ecmascript::JSFunction::ClassConstructorBit::SIZE) - 1)),
            GetWord64Constant(0));
    }

    AddrShift IsExtensible(AddrShift object)
    {
        AddrShift hClass = LoadHClass(object);
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);

        AddrShift bitfield = Load(MachineType::INT64_TYPE, hClass, bitfieldOffset);
        // decode
        return Word64NotEqual(
            Word64And(Word64LSR(bitfield, GetWord64Constant(panda::ecmascript::JSHClass::ExtensibleBit::START_BIT)),
                      GetWord64Constant((1LLU << panda::ecmascript::JSHClass::ExtensibleBit::SIZE) - 1)),
            GetWord64Constant(0));
    }

    AddrShift IsSymbol(AddrShift obj)
    {
        AddrShift objectType = GetObjectType(LoadHClass(obj));
        return Word32Equal(objectType, GetInt32Constant(static_cast<int32_t>(panda::ecmascript::JSType::SYMBOL)));
    }

    AddrShift IsString(AddrShift obj)
    {
        AddrShift objectType = GetObjectType(LoadHClass(obj));
        return Word32Equal(objectType, GetInt32Constant(static_cast<int32_t>(panda::ecmascript::JSType::STRING)));
    }

    AddrShift IsJsProxy(AddrShift obj)
    {
        AddrShift objectType = GetObjectType(LoadHClass(obj));
        return Word32Equal(objectType, GetInt32Constant(static_cast<int32_t>(panda::ecmascript::JSType::JS_PROXY)));
    }

    AddrShift IsJsArray(AddrShift obj)
    {
        AddrShift objectType = GetObjectType(LoadHClass(obj));
        return Word32Equal(objectType, GetInt32Constant(static_cast<int32_t>(panda::ecmascript::JSType::JS_ARRAY)));
    }

    AddrShift IsWritable(AddrShift attr)
    {
        return Word32NotEqual(
            Word32And(
                Word32LSR(attr, GetInt32Constant(panda::ecmascript::PropertyAttributes::WritableField::START_BIT)),
                GetInt32Constant((1LLU << panda::ecmascript::PropertyAttributes::WritableField::SIZE) - 1)),
            GetInt32Constant(0));
    }

    AddrShift IsAccessor(AddrShift attr)
    {
        return Word32NotEqual(
            Word32And(Word32LSR(attr,
                GetInt32Constant(panda::ecmascript::PropertyAttributes::IsAccessorField::START_BIT)),
                GetInt32Constant((1LLU << panda::ecmascript::PropertyAttributes::IsAccessorField::SIZE) - 1)),
            GetInt32Constant(0));
    }

    AddrShift IsInlinedProperty(AddrShift attr)
    {
        return Word32NotEqual(
            Word32And(Word32LSR(attr,
                GetInt32Constant(panda::ecmascript::PropertyAttributes::IsInlinedPropsField::START_BIT)),
                GetInt32Constant((1LLU << panda::ecmascript::PropertyAttributes::IsInlinedPropsField::SIZE) - 1)),
            GetInt32Constant(0));
    }

    AddrShift PropAttrGetOffset(AddrShift attr)
    {
        return Word32And(
            Word32LSR(attr, GetInt32Constant(panda::ecmascript::PropertyAttributes::OffsetField::START_BIT)),
            GetInt32Constant((1LLU << panda::ecmascript::PropertyAttributes::OffsetField::SIZE) - 1));
    }

    // SetDictionaryOrder func in property_attribute.h
    AddrShift SetDictionaryOrderFieldInPropAttr(AddrShift attr, AddrShift value)
    {
        AddrShift mask = Word32LSL(
            GetInt32Constant((1LLU << panda::ecmascript::PropertyAttributes::DictionaryOrderField::SIZE) - 1),
            GetInt32Constant(panda::ecmascript::PropertyAttributes::DictionaryOrderField::START_BIT));
        AddrShift newVal = Word32Or(Word32And(attr, Word32Not(mask)),
            Word32LSL(value, GetInt32Constant(panda::ecmascript::PropertyAttributes::DictionaryOrderField::START_BIT)));
        return newVal;
    }

    AddrShift GetPrototypeFromHClass(AddrShift hClass)
    {
        AddrShift protoOffset = GetPtrConstant(panda::ecmascript::JSHClass::PROTOTYPE_OFFSET);
        return Load(MachineType::TAGGED_TYPE, hClass, protoOffset);
    }

    AddrShift GetAttributesFromHclass(AddrShift hClass)
    {
        AddrShift attrOffset = GetPtrConstant(panda::ecmascript::JSHClass::ATTRIBUTES_OFFSET);
        return Load(MachineType::TAGGED_TYPE, hClass, attrOffset);
    }

    AddrShift GetPropertiesNumberFromHClass(AddrShift hClass)
    {
        AddrShift bitfield = Load(MachineType::INT64_TYPE, hClass,
            GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET));
        AddrShift unusedNonInlinedProps = Word64And(Word64LSR(bitfield,
            GetWord64Constant(panda::ecmascript::JSHClass::NumberOfUnusedNonInlinedPropsBits::START_BIT)),
            GetWord64Constant((1LLU << panda::ecmascript::JSHClass::NumberOfUnusedNonInlinedPropsBits::SIZE) - 1));
        AddrShift unusedInlinedProps = Word64And(Word64LSR(bitfield,
            GetWord64Constant(panda::ecmascript::JSHClass::NumberOfUnusedInlinedPropsBits::START_BIT)),
            GetWord64Constant((1LLU << panda::ecmascript::JSHClass::NumberOfUnusedInlinedPropsBits::SIZE) - 1));
        return Int64Sub(Int64Sub(GetWord64Constant(
            panda::ecmascript::PropertyAttributes::MAX_CAPACITY_OF_PROPERTIES),
            unusedNonInlinedProps),
            unusedInlinedProps);
    }

    AddrShift GetObjectSizeFromHClass(AddrShift hClass) // NOTE: need to add special case for string and TAGGED_ARRAY
    {
        return Load(MachineType::UINT64_TYPE, hClass, GetPtrConstant(panda::ecmascript::JSHClass::OBJECT_SIZE_OFFSET));
    }

    void StoreElement(AddrShift thread, AddrShift elements, AddrShift index, AddrShift value)
    {
        AddrShift offset =
            PtrMul(ChangeInt32ToPointer(index), GetPtrConstant(panda::ecmascript::JSTaggedValue::TaggedTypeSize()));
        AddrShift dataOffset = PtrAdd(offset, GetPtrConstant(panda::coretypes::Array::GetDataOffset()));
        Store(MachineType::TAGGED_TYPE, thread, elements, dataOffset, value);
    }

    void StoreElement(AddrShift elements, AddrShift index, AddrShift value)
    {
        AddrShift offset =
            PtrMul(ChangeInt32ToPointer(index), GetPtrConstant(panda::ecmascript::JSTaggedValue::TaggedTypeSize()));
        AddrShift dataOffset = PtrAdd(offset, GetPtrConstant(panda::coretypes::Array::GetDataOffset()));
        Store(MachineType::TAGGED_TYPE, elements, dataOffset, value);
    }

    void ThrowTypeAndReturn(AddrShift thread, int messageId, AddrShift val);

    AddrShift GetValueFromTaggedArray(AddrShift elements, AddrShift index)
    {
        AddrShift offset =
            PtrMul(ChangeInt32ToPointer(index), GetPtrConstant(panda::ecmascript::JSTaggedValue::TaggedTypeSize()));
        AddrShift dataOffset = PtrAdd(offset, GetPtrConstant(panda::coretypes::Array::GetDataOffset()));
        return Load(MachineType::TAGGED_TYPE, elements, dataOffset);
    }

    AddrShift TaggedToRepresentation(AddrShift value);

    AddrShift GetElementRepresentation(AddrShift hClass)
    {
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);
        AddrShift bitfield = Load(MachineType::INT64_TYPE, hClass, bitfieldOffset);
        return Word64And(
            Word64LSR(bitfield, GetWord64Constant(panda::ecmascript::JSHClass::ElementRepresentationBits::START_BIT)),
            GetWord64Constant(((1LLU << panda::ecmascript::JSHClass::ElementRepresentationBits::SIZE) - 1)));
    }

    void SetElementRepresentation(AddrShift hClass, AddrShift value)
    {
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);
        AddrShift oldValue = Load(MachineType::INT64_TYPE, hClass, bitfieldOffset);
        AddrShift oldWithMask = Word64And(oldValue,
            GetWord64Constant(~panda::ecmascript::JSHClass::ElementRepresentationBits::Mask()));
        AddrShift newValue = Word64LSR(value, GetWord64Constant(
            panda::ecmascript::JSHClass::ElementRepresentationBits::START_BIT));
        Store(MachineType::INT64_TYPE, hClass, bitfieldOffset, Word64Or(oldWithMask, newValue));
    }

    void UpdateValueAndAttributes(AddrShift thread, AddrShift elements, AddrShift index,
        AddrShift value, AddrShift attr)
    {
        AddrShift arrayIndex =
            Int32Add(GetInt32Constant(panda::ecmascript::NameDictionary::TABLE_HEADER_SIZE),
                     Int32Mul(index, GetInt32Constant(panda::ecmascript::NameDictionary::ENTRY_SIZE)));
        AddrShift valueIndex =
            Int32Add(arrayIndex, GetInt32Constant(panda::ecmascript::NameDictionary::ENTRY_VALUE_INDEX));
        AddrShift attributesIndex =
            Int32Add(arrayIndex, GetInt32Constant(panda::ecmascript::NameDictionary::ENTRY_DETAILS_INDEX));
        StoreElement(elements, valueIndex, value);
        StoreElement(elements, attributesIndex, IntBuildTagged(attr));
    }

    AddrShift IsSpecialIndexedObj(AddrShift jsType)
    {
        return Int32GreaterThan(jsType,
                                GetInt32Constant(static_cast<int32_t>(panda::ecmascript::JSType::JS_ARRAY)));
    }

    AddrShift IsAccessorInternal(AddrShift value)
    {
        return Word32Equal(GetObjectType(LoadHClass(value)),
                           GetInt32Constant(static_cast<int32_t>(panda::ecmascript::JSType::INTERNAL_ACCESSOR)));
    }

    void UpdateAndStoreRepresention(AddrShift hClass, AddrShift value);

    AddrShift UpdateRepresention(AddrShift oldRep, AddrShift value);

    AddrShift GetAttributesFromDictionary(AddrShift elements, AddrShift entry)
    {
        AddrShift arrayIndex =
            Int32Add(GetInt32Constant(panda::ecmascript::NumberDictionary::TABLE_HEADER_SIZE),
                     Int32Mul(entry, GetInt32Constant(panda::ecmascript::NumberDictionary::ENTRY_SIZE)));
        AddrShift attributesIndex =
            Int32Add(arrayIndex, GetInt32Constant(panda::ecmascript::NameDictionary::ENTRY_DETAILS_INDEX));
        return TaggedCastToInt32(GetValueFromTaggedArray(elements, attributesIndex));
    }

    AddrShift GetValueFromDictionary(AddrShift elements, AddrShift entry)
    {
        AddrShift arrayIndex =
            Int32Add(GetInt32Constant(panda::ecmascript::NumberDictionary::TABLE_HEADER_SIZE),
                     Int32Mul(entry, GetInt32Constant(panda::ecmascript::NumberDictionary::ENTRY_SIZE)));
        AddrShift valueIndex =
            Int32Add(arrayIndex, GetInt32Constant(panda::ecmascript::NameDictionary::ENTRY_VALUE_INDEX));
        return GetValueFromTaggedArray(elements, valueIndex);
    }

    AddrShift GetKeyFromNumberDictionary(AddrShift elements, AddrShift entry);

    AddrShift GetPropAttrFromLayoutInfo(AddrShift layout, AddrShift entry)
    {
        AddrShift index = Int32Add(
            Int32Add(GetInt32Constant(panda::ecmascript::LayoutInfo::ELEMENTS_START_INDEX),
                Word32LSL(entry, GetInt32Constant(1))),
            GetInt32Constant(1));
        return GetValueFromTaggedArray(layout, index);
    }

    AddrShift IsMatchInNumberDictionary(AddrShift key, AddrShift other);

    AddrShift FindElementFromNumberDictionary(AddrShift thread, AddrShift elements, AddrShift key);

    AddrShift FindEntryFromNameDictionary(AddrShift thread, AddrShift elements, AddrShift key);

    AddrShift JSObjectGetProperty(AddrShift obj, AddrShift hClass, AddrShift propAttr);

    AddrShift IsUtf16String(AddrShift string);

    AddrShift IsUtf8String(AddrShift string);

    AddrShift IsInternalString(AddrShift string);

    AddrShift IsDigit(AddrShift ch);

    AddrShift StringToElementIndex(AddrShift string);

    AddrShift TryToElementsIndex(AddrShift key);

    AddrShift TaggedCastToInt64(AddrShift x)
    {
        return Word64And(x, GetWord64Constant(~panda::ecmascript::JSTaggedValue::TAG_MASK));
    }

    AddrShift TaggedCastToInt32(AddrShift x)
    {
        return ChangeInt64ToInt32(TaggedCastToInt64(x));
    }

    AddrShift TaggedCastToDouble(AddrShift x)
    {
        AddrShift val = Int64Sub(x, GetWord64Constant(panda::ecmascript::JSTaggedValue::DOUBLE_ENCODE_OFFSET));
        return CastInt64ToFloat64(val);
    }

    AddrShift ChangeInt32ToFloat64(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::INT32_TO_FLOAT64), x);
    }

    AddrShift ChangeFloat64ToInt32(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::FLOAT64_TO_INT32), x);
    }

    AddrShift CastInt64ToFloat64(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::BITCAST_INT64_TO_FLOAT64), x);
    }

    AddrShift SExtInt32ToInt64(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::SEXT_INT32_TO_INT64), x);
    }

    AddrShift SExtInt1ToInt64(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::SEXT_INT1_TO_INT64), x);
    }

    AddrShift SExtInt1ToInt32(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::SEXT_INT1_TO_INT32), x);
    }

    AddrShift ZExtInt32ToInt64(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::ZEXT_INT32_TO_INT64), x);
    }

    AddrShift ZExtInt1ToInt64(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::ZEXT_INT1_TO_INT64), x);
    }

    AddrShift ZExtInt1ToInt32(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::ZEXT_INT1_TO_INT32), x);
    }

    AddrShift ZExtInt8ToInt32(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::ZEXT_INT8_TO_INT32), x);
    }

    AddrShift ZExtInt16ToInt32(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::ZEXT_INT16_TO_INT32), x);
    }

    AddrShift TruncInt64ToInt32(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::TRUNC_INT64_TO_INT32), x);
    }

    AddrShift TruncInt64ToInt1(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::TRUNC_INT64_TO_INT1), x);
    }

    AddrShift TruncInt32ToInt1(AddrShift x)
    {
        return env_.GetCircuitBuilder().NewArithMeticGate(OpCode(OpCode::TRUNC_INT32_TO_INT1), x);
    }

    int NextVariableId()
    {
        return nextVariableId_++;
    }

    std::string GetMethodName() const
    {
        return methodName_;
    }

    AddrShift GetGlobalConstantAddr(AddrShift index)
    {
        return Int64Mul(GetWord64Constant(sizeof(JSTaggedValue)), index);
    }

    AddrShift GetGlobalConstantString(ConstantIndex index)
    {
        return GetGlobalConstantAddr(GetWord64Constant(static_cast<int>(index)));
    }

    AddrShift IsCallable(AddrShift obj)
    {
        AddrShift hclass = LoadHClass(obj);
        AddrShift bitfieldOffset = GetPtrConstant(panda::ecmascript::JSHClass::BIT_FIELD_OFFSET);

        AddrShift bitfield = Load(MachineType::INT64_TYPE, hclass, bitfieldOffset);
        // decode
        return Word64NotEqual(
            Word64And(Word64LSR(bitfield, GetWord64Constant(panda::ecmascript::JSHClass::CallableBit::START_BIT)),
                      GetWord64Constant((1LLU << panda::ecmascript::JSHClass::CallableBit::SIZE) - 1)),
            GetWord64Constant(0));
    }

private:
    Environment env_;
    std::string methodName_;
    int nextVariableId_ {0};
};
}  // namespace kungfu
#endif  // ECMASCRIPT_COMPILER_Stub_H
