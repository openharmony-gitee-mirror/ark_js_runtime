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

#include <codecvt>
#include <iomanip>
#include <iostream>
#include <string>

#include "ecmascript/accessor_data.h"
#include "ecmascript/class_linker/program_object-inl.h"
#include "ecmascript/ecma_module.h"
#include "ecmascript/ecma_vm.h"
#include "ecmascript/global_dictionary-inl.h"
#include "ecmascript/global_env.h"
#include "ecmascript/ic/ic_handler.h"
#include "ecmascript/ic/proto_change_details.h"
#include "ecmascript/ic/property_box.h"
#include "ecmascript/interpreter/frame_handler.h"
#include "ecmascript/jobs/micro_job_queue.h"
#include "ecmascript/jobs/pending_job.h"
#include "ecmascript/js_array.h"
#include "ecmascript/js_array_iterator.h"
#include "ecmascript/js_arraybuffer.h"
#include "ecmascript/js_async_function.h"
#include "ecmascript/js_collator.h"
#include "ecmascript/js_dataview.h"
#include "ecmascript/js_date.h"
#include "ecmascript/js_date_time_format.h"
#include "ecmascript/js_for_in_iterator.h"
#include "ecmascript/js_function.h"
#include "ecmascript/js_function_extra_info.h"
#include "ecmascript/js_generator_object.h"
#include "ecmascript/js_global_object.h"
#include "ecmascript/js_handle.h"
#include "ecmascript/js_intl.h"
#include "ecmascript/js_locale.h"
#include "ecmascript/js_map.h"
#include "ecmascript/js_map_iterator.h"
#include "ecmascript/js_number_format.h"
#include "ecmascript/js_object-inl.h"
#include "ecmascript/js_plural_rules.h"
#include "ecmascript/js_primitive_ref.h"
#include "ecmascript/js_promise.h"
#include "ecmascript/js_realm.h"
#include "ecmascript/js_regexp.h"
#include "ecmascript/js_relative_time_format.h"
#include "ecmascript/js_set.h"
#include "ecmascript/js_set_iterator.h"
#include "ecmascript/js_string_iterator.h"
#include "ecmascript/js_tagged_number.h"
#include "ecmascript/js_tagged_value-inl.h"
#include "ecmascript/js_thread.h"
#include "ecmascript/js_typed_array.h"
#include "ecmascript/js_weak_container.h"
#include "ecmascript/layout_info-inl.h"
#include "ecmascript/lexical_env.h"
#include "ecmascript/linked_hash_table-inl.h"
#include "ecmascript/mem/assert_scope-inl.h"
#include "ecmascript/mem/c_containers.h"
#include "ecmascript/mem/machine_code.h"
#include "ecmascript/tagged_array.h"
#include "ecmascript/tagged_dictionary.h"
#include "ecmascript/template_map.h"
#include "ecmascript/transitions_dictionary.h"

namespace panda::ecmascript {
using MicroJobQueue = panda::ecmascript::job::MicroJobQueue;
using PendingJob = panda::ecmascript::job::PendingJob;

static constexpr uint32_t DUMP_TYPE_OFFSET = 12;
static constexpr uint32_t DUMP_PROPERTY_OFFSET = 20;

CString JSHClass::DumpJSType(JSType type)
{
    switch (type) {
        case JSType::HCLASS:
            return "JSHClass";
        case JSType::TAGGED_ARRAY:
            return "TaggedArray";
        case JSType::TAGGED_DICTIONARY:
            return "TaggedDictionary";
        case JSType::STRING:
            return "BaseString";
        case JSType::JS_NATIVE_POINTER:
            return "NativePointer";
        case JSType::JS_OBJECT:
            return "Object";
        case JSType::JS_FUNCTION_BASE:
            return "Function Base";
        case JSType::JS_FUNCTION:
            return "Function";
        case JSType::JS_ERROR:
            return "Error";
        case JSType::JS_EVAL_ERROR:
            return "Eval Error";
        case JSType::JS_RANGE_ERROR:
            return "Range Error";
        case JSType::JS_TYPE_ERROR:
            return "Type Error";
        case JSType::JS_REFERENCE_ERROR:
            return "Reference Error";
        case JSType::JS_URI_ERROR:
            return "Uri Error";
        case JSType::JS_SYNTAX_ERROR:
            return "Syntax Error";
        case JSType::JS_REG_EXP:
            return "Regexp";
        case JSType::JS_SET:
            return "Set";
        case JSType::JS_MAP:
            return "Map";
        case JSType::JS_WEAK_SET:
            return "WeakSet";
        case JSType::JS_WEAK_MAP:
            return "WeakMap";
        case JSType::JS_DATE:
            return "Date";
        case JSType::JS_BOUND_FUNCTION:
            return "Bound Function";
        case JSType::JS_ARRAY:
            return "Array";
        case JSType::JS_TYPED_ARRAY:
            return "Typed Array";
        case JSType::JS_INT8_ARRAY:
            return "Int8 Array";
        case JSType::JS_UINT8_ARRAY:
            return "Uint8 Array";
        case JSType::JS_UINT8_CLAMPED_ARRAY:
            return "Uint8 Clamped Array";
        case JSType::JS_INT16_ARRAY:
            return "Int16 Array";
        case JSType::JS_UINT16_ARRAY:
            return "Uint16 Array";
        case JSType::JS_INT32_ARRAY:
            return "Int32 Array";
        case JSType::JS_UINT32_ARRAY:
            return "Uint32 Array";
        case JSType::JS_FLOAT32_ARRAY:
            return "Float32 Array";
        case JSType::JS_FLOAT64_ARRAY:
            return "Float64 Array";
        case JSType::JS_ARGUMENTS:
            return "Arguments";
        case JSType::JS_PROXY:
            return "Proxy";
        case JSType::JS_PRIMITIVE_REF:
            return "Primitive";
        case JSType::JS_DATA_VIEW:
            return "DataView";
        case JSType::JS_ITERATOR:
            return "Iterator";
        case JSType::JS_FORIN_ITERATOR:
            return "ForinInterator";
        case JSType::JS_MAP_ITERATOR:
            return "MapIterator";
        case JSType::JS_SET_ITERATOR:
            return "SetIterator";
        case JSType::JS_ARRAY_ITERATOR:
            return "ArrayIterator";
        case JSType::JS_STRING_ITERATOR:
            return "StringIterator";
        case JSType::JS_ARRAY_BUFFER:
            return "ArrayBuffer";
        case JSType::JS_PROXY_REVOC_FUNCTION:
            return "ProxyRevocFunction";
        case JSType::PROMISE_REACTIONS:
            return "PromiseReaction";
        case JSType::PROMISE_CAPABILITY:
            return "PromiseCapability";
        case JSType::PROMISE_ITERATOR_RECORD:
            return "PromiseIteratorRecord";
        case JSType::PROMISE_RECORD:
            return "PromiseRecord";
        case JSType::RESOLVING_FUNCTIONS_RECORD:
            return "ResolvingFunctionsRecord";
        case JSType::JS_PROMISE:
            return "Promise";
        case JSType::JS_PROMISE_REACTIONS_FUNCTION:
            return "PromiseReactionsFunction";
        case JSType::JS_PROMISE_EXECUTOR_FUNCTION:
            return "PromiseExecutorFunction";
        case JSType::JS_PROMISE_ALL_RESOLVE_ELEMENT_FUNCTION:
            return "PromiseAllResolveElementFunction";
        case JSType::MICRO_JOB_QUEUE:
            return "MicroJobQueue";
        case JSType::PENDING_JOB:
            return "PendingJob";
        case JSType::COMPLETION_RECORD:
            return "CompletionRecord";
        case JSType::GLOBAL_ENV:
            return "GlobalEnv";
        case JSType::ACCESSOR_DATA:
            return "AccessorData";
        case JSType::INTERNAL_ACCESSOR:
            return "InternalAccessor";
        case JSType::SYMBOL:
            return "Symbol";
        case JSType::PROPERTY_BOX:
            return "PropertyBox";
        case JSType::JS_ASYNC_FUNCTION:
            return "AsyncFunction";
        case JSType::JS_ASYNC_AWAIT_STATUS_FUNCTION:
            return "AsyncAwaitStatusFunction";
        case JSType::JS_ASYNC_FUNC_OBJECT:
            return "AsyncFunctionObject";
        case JSType::JS_REALM:
            return "Realm";
        case JSType::JS_GLOBAL_OBJECT:
            return "GlobalObject";
        case JSType::JS_INTL:
            return "JSIntl";
        case JSType::JS_LOCALE:
            return "JSLocale";
        case JSType::JS_DATE_TIME_FORMAT:
            return "JSDateTimeFormat";
        case JSType::JS_RELATIVE_TIME_FORMAT:
            return "JSRelativeTimeFormat";
        case JSType::JS_NUMBER_FORMAT:
            return "JSNumberFormat";
        case JSType::JS_COLLATOR:
            return "JSCollator";
        case JSType::JS_PLURAL_RULES:
            return "JSPluralRules";
        case JSType::JS_GENERATOR_OBJECT:
            return "JSGeneratorObject";
        case JSType::JS_GENERATOR_CONTEXT:
            return "JSGeneratorContext";
        case JSType::PROTO_CHANGE_MARKER:
            return "ProtoChangeMarker";
        case JSType::PROTOTYPE_INFO:
            return "PrototypeInfo";
        case JSType::PROGRAM:
            return "program";
        case JSType::LEXICAL_FUNCTION:
            return "LexicalFunction";
        case JSType::FUNCTION_EXTRA_INFO:
            return "FunctionExtraInfo";
        case JSType::MACHINE_CODE_OBJECT:
            return "MachineCode";
        case JSType::ECMA_MODULE:
            return "EcmaModule";
        default: {
            CString ret = "unknown type ";
            return ret + static_cast<char>(type);
        }
    }
}

static void DumpArrayClass(JSThread *thread, const TaggedArray *arr, std::ostream &os)
{
    DISALLOW_GARBAGE_COLLECTION;
    array_size_t len = arr->GetLength();
    os << " <TaggedArray[" << std::dec << len << "]>\n";
    for (array_size_t i = 0; i < len; i++) {
        JSTaggedValue val(arr->Get(i));
        if (!val.IsHole()) {
            os << std::right << std::setw(DUMP_PROPERTY_OFFSET) << i << ": ";
            val.DumpTaggedValue(thread, os);
            os << "\n";
        }
    }
}

static void DumpStringClass(const EcmaString *str, std::ostream &os)
{
    DISALLOW_GARBAGE_COLLECTION;
    CString string = ConvertToString(str);
    os << string;
}

static void DumpPropertyKey(JSTaggedValue key, std::ostream &os)
{
    if (key.IsString()) {
        DumpStringClass(EcmaString::Cast(key.GetTaggedObject()), os);
    } else if (key.IsSymbol()) {
        JSSymbol *sym = JSSymbol::Cast(key.GetTaggedObject());
        DumpStringClass(EcmaString::Cast(sym->GetDescription().GetTaggedObject()), os);
    } else {
        UNREACHABLE();
    }
}

static void DumpHClass(JSThread *thread, const JSHClass *jshclass, std::ostream &os, bool withDetail)
{
    DISALLOW_GARBAGE_COLLECTION;
    os << "JSHClass :" << std::setw(DUMP_TYPE_OFFSET);
    os << "Type :" << JSHClass::DumpJSType(jshclass->GetObjectType()) << "\n";

    os << " - Prototype :" << std::setw(DUMP_TYPE_OFFSET);
    jshclass->GetPrototype().DumpTaggedValue(thread, os);
    os << "\n";
    os << " - PropertyDescriptors :" << std::setw(DUMP_TYPE_OFFSET);
    JSTaggedValue attrs = jshclass->GetAttributes();
    attrs.DumpTaggedValue(thread, os);
    os << "\n";
    if (withDetail && !attrs.IsNull()) {
        LayoutInfo *layoutInfo = LayoutInfo::Cast(attrs.GetTaggedObject());
        layoutInfo->Dump(thread, os);
    }
    os << " - Transitions :" << std::setw(DUMP_TYPE_OFFSET);
    JSTaggedValue transtions = jshclass->GetTransitions();
    transtions.DumpTaggedValue(thread, os);
    os << "\n";
    if (withDetail && !transtions.IsNull()) {
        transtions.Dump(thread, os);
    }
    os << " - Parent :" << std::setw(DUMP_TYPE_OFFSET);
    jshclass->GetParent().DumpTaggedValue(thread, os);
    os << "\n";

    os << " - Flags : " << std::setw(DUMP_TYPE_OFFSET);
    os << "Ctor :" << jshclass->IsConstructor();
    os << "| Callable :" << jshclass->IsCallable();
    os << "| Extensible :" << jshclass->IsExtensible();
    os << "| ElementRepresentation :" << static_cast<int>(jshclass->GetElementRepresentation());
    os << "| UnusedInlineProperties :" << std::dec << jshclass->GetUnusedInlinedProps();
    os << "| UnusedOutProperties :" << std::dec << jshclass->GetUnusedNonInlinedProps();
    os << "\n";
}

static void DumpDynClass(JSThread *thread, TaggedObject *obj, std::ostream &os)
{
    JSHClass *hclass = obj->GetClass();
    os << "JSHClass :" << std::setw(DUMP_TYPE_OFFSET) << " klass_(" << std::hex << hclass << ")\n";
    DumpHClass(thread, hclass, os, true);
}

static void DumpAttr(const PropertyAttributes &attr, bool fastMode, std::ostream &os)
{
    if (attr.IsAccessor()) {
        os << "(Accessor) ";
    }

    os << "Attr(";
    if (attr.IsNoneAttributes()) {
        os << "NONE";
    }
    if (attr.IsWritable()) {
        os << "W";
    }
    if (attr.IsEnumerable()) {
        os << "E";
    }
    if (attr.IsConfigurable()) {
        os << "C";
    }
    os << ")";

    os << " InlinedProps: " << attr.IsInlinedProps();

    if (fastMode) {
        os << " Order: " << std::dec << attr.GetOffset();
        os << " SortedIndex: " << std::dec << attr.GetSortedIndex();
    } else {
        os << " Order: " << std::dec << attr.GetDictionaryOrder();
    }
}

static void DumpObject(JSThread *thread, TaggedObject *obj, std::ostream &os)
{
    DISALLOW_GARBAGE_COLLECTION;
    auto jsHclass = obj->GetClass();
    JSType type = jsHclass->GetObjectType();

    switch (type) {
        case JSType::HCLASS:
            return DumpDynClass(thread, obj, os);
        case JSType::TAGGED_ARRAY:
        case JSType::TAGGED_DICTIONARY:
        case JSType::TEMPLATE_MAP:
            DumpArrayClass(thread, TaggedArray::Cast(obj), os);
            break;
        case JSType::STRING:
            DumpStringClass(EcmaString::Cast(obj), os);
            os << "\n";
            break;
        case JSType::JS_NATIVE_POINTER:
            break;
        case JSType::JS_OBJECT:
        case JSType::JS_GLOBAL_OBJECT:
        case JSType::JS_ERROR:
        case JSType::JS_EVAL_ERROR:
        case JSType::JS_RANGE_ERROR:
        case JSType::JS_TYPE_ERROR:
        case JSType::JS_REFERENCE_ERROR:
        case JSType::JS_URI_ERROR:
        case JSType::JS_SYNTAX_ERROR:
        case JSType::JS_ARGUMENTS:
        case JSType::JS_FUNCTION_BASE:
            JSObject::Cast(obj)->Dump(thread, os);
            break;
        case JSType::GLOBAL_ENV:
            GlobalEnv::Cast(obj)->Dump(thread, os);
            break;
        case JSType::ACCESSOR_DATA:
            break;
        case JSType::JS_FUNCTION:
            JSFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_BOUND_FUNCTION:
            JSBoundFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_SET:
            JSSet::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_MAP:
            JSMap::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_WEAK_SET:
            JSWeakSet::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_WEAK_MAP:
            JSWeakMap::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_REG_EXP:
            JSRegExp::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_DATE:
            JSDate::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_ARRAY:
            JSArray::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_TYPED_ARRAY:
        case JSType::JS_INT8_ARRAY:
        case JSType::JS_UINT8_ARRAY:
        case JSType::JS_UINT8_CLAMPED_ARRAY:
        case JSType::JS_INT16_ARRAY:
        case JSType::JS_UINT16_ARRAY:
        case JSType::JS_INT32_ARRAY:
        case JSType::JS_UINT32_ARRAY:
        case JSType::JS_FLOAT32_ARRAY:
        case JSType::JS_FLOAT64_ARRAY:
            JSTypedArray::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PROXY:
            JSProxy::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PRIMITIVE_REF:
            JSPrimitiveRef::Cast(obj)->Dump(thread, os);
            break;
        case JSType::SYMBOL:
            JSSymbol::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_DATA_VIEW:
            JSDataView::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_ARRAY_BUFFER:
            JSArrayBuffer::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROMISE_REACTIONS:
            PromiseReaction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROMISE_CAPABILITY:
            PromiseCapability::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROMISE_ITERATOR_RECORD:
            PromiseIteratorRecord::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROMISE_RECORD:
            PromiseRecord::Cast(obj)->Dump(thread, os);
            break;
        case JSType::RESOLVING_FUNCTIONS_RECORD:
            ResolvingFunctionsRecord::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PROMISE:
            JSPromise::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PROMISE_REACTIONS_FUNCTION:
            JSPromiseReactionsFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PROMISE_EXECUTOR_FUNCTION:
            JSPromiseExecutorFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PROMISE_ALL_RESOLVE_ELEMENT_FUNCTION:
            JSPromiseAllResolveElementFunction::Cast(obj)->Dump(thread);
            break;
        case JSType::MICRO_JOB_QUEUE:
            MicroJobQueue::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PENDING_JOB:
            PendingJob::Cast(obj)->Dump(thread, os);
            break;
        case JSType::COMPLETION_RECORD:
            CompletionRecord::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PROXY_REVOC_FUNCTION:
            JSProxyRevocFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_ASYNC_FUNCTION:
            JSAsyncFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_ASYNC_AWAIT_STATUS_FUNCTION:
            JSAsyncAwaitStatusFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_GENERATOR_FUNCTION:
            JSGeneratorFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_INTL_BOUND_FUNCTION:
            JSIntlBoundFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_ITERATOR:
            break;
        case JSType::JS_FORIN_ITERATOR:
            JSForInIterator::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_MAP_ITERATOR:
            JSMapIterator::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_SET_ITERATOR:
            JSSetIterator::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_ARRAY_ITERATOR:
            JSArrayIterator::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_STRING_ITERATOR:
            JSStringIterator::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROTOTYPE_HANDLER:
            PrototypeHandler::Cast(obj)->Dump(thread, os);
            break;
        case JSType::TRANSITION_HANDLER:
            TransitionHandler::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROPERTY_BOX:
            PropertyBox::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_REALM:
            JSRealm::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_INTL:
            JSIntl::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_LOCALE:
            JSLocale::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_DATE_TIME_FORMAT:
            JSDateTimeFormat::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_RELATIVE_TIME_FORMAT:
            JSRelativeTimeFormat::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_NUMBER_FORMAT:
            JSNumberFormat::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_COLLATOR:
            JSCollator::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_PLURAL_RULES:
            JSPluralRules::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_GENERATOR_OBJECT:
            JSGeneratorObject::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_ASYNC_FUNC_OBJECT:
            JSAsyncFuncObject::Cast(obj)->Dump(thread, os);
            break;
        case JSType::JS_GENERATOR_CONTEXT:
            GeneratorContext::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROTOTYPE_INFO:
            ProtoChangeDetails::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROTO_CHANGE_MARKER:
            ProtoChangeMarker::Cast(obj)->Dump(thread, os);
            break;
        case JSType::PROGRAM:
            Program::Cast(obj)->Dump(thread, os);
            break;
        case JSType::LEXICAL_FUNCTION:
            LexicalFunction::Cast(obj)->Dump(thread, os);
            break;
        case JSType::FUNCTION_EXTRA_INFO:
            JSFunctionExtraInfo::Cast(obj)->Dump(thread, os);
            break;
        case JSType::MACHINE_CODE_OBJECT:
            MachineCode::Cast(obj)->Dump(thread, os);
            break;
        case JSType::ECMA_MODULE:
            EcmaModule::Cast(obj)->Dump(thread, os);
            break;
        default:
            UNREACHABLE();
            break;
    }

    DumpHClass(thread, jsHclass, os, false);
}

void JSTaggedValue::DumpSpecialValue([[maybe_unused]] JSThread *thread, std::ostream &os) const
{
    ASSERT(IsSpecial());
    os << "[Special Value] : ";
    switch (GetRawData()) {
        case VALUE_HOLE:
            os << "Hole";
            break;
        case VALUE_NULL:
            os << "Null";
            break;
        case VALUE_FALSE:
            os << "False";
            break;
        case VALUE_TRUE:
            os << "True";
            break;
        case VALUE_UNDEFINED:
            os << "Undefined";
            break;
        case VALUE_EXCEPTION:
            os << "Exception";
            break;
        default:
            UNREACHABLE();
            break;
    }
}

void JSTaggedValue::DumpHeapObjectType([[maybe_unused]] JSThread *thread, std::ostream &os) const
{
    ASSERT(IsWeak() || IsHeapObject());
    bool isWeak = IsWeak();
    TaggedObject *obj = isWeak ? GetTaggedWeakRef() : GetTaggedObject();
    if (isWeak) {
        os << "----------Dump Weak Referent----------"
           << "\n";
    }

    JSType type = GetTaggedObject()->GetClass()->GetObjectType();
    if (type == JSType::STRING) {
        CString string = ConvertToString(EcmaString::Cast(obj));
        os << std::left << std::setw(DUMP_TYPE_OFFSET) << "[" + string + "]";
    } else {
        std::ostringstream address;
        address << obj;
        CString addrStr = CString(address.str());

        os << std::left << std::setw(DUMP_TYPE_OFFSET) << "[" + JSHClass::DumpJSType(type) + "(" + addrStr + ")]";
    }
}

void JSTaggedValue::DumpTaggedValue(JSThread *thread, std::ostream &os) const
{
    if (IsInt()) {
        os << std::left << std::setw(DUMP_TYPE_OFFSET) << "[Int] : " << std::hex << "0x" << GetInt() << std::dec << " ("
           << GetInt() << ")";
    } else if (IsDouble()) {
        os << std::left << std::setw(DUMP_TYPE_OFFSET) << "[Double] : " << GetDouble();
    } else if (IsSpecial()) {
        DumpSpecialValue(thread, os);
    } else {
        DumpHeapObjectType(thread, os);
    }
}

void JSTaggedValue::Dump(JSThread *thread, std::ostream &os) const
{
    DumpTaggedValue(thread, os);
    os << "\n";

    if (IsHeapObject()) {
        TaggedObject *obj = GetTaggedObject();
        DumpObject(thread, obj, os);
    }
}

void JSTaggedValue::Dump(JSThread *thread) const
{
    Dump(thread, std::cout);
}

void JSTaggedValue::DumpVal(JSThread *thread, JSTaggedType val)
{
    JSTaggedValue(val).Dump(thread);
}

void JSThread::DumpStack()
{
    InterpretedFrameHandler handler(this);
    handler.DumpStack(std::cout);
}

void NumberDictionary::Dump(JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int size = Size();
    for (int hashIndex = 0; hashIndex < size; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole()) {
            JSTaggedValue val(GetValue(hashIndex));
            os << std::right << std::setw(DUMP_PROPERTY_OFFSET)
               << static_cast<uint32_t>(JSTaggedNumber(key).GetNumber()) << ": ";
            val.DumpTaggedValue(thread, os);
            os << " ";
            DumpAttr(GetAttributes(hashIndex), false, os);
            os << "\n";
        }
    }
}

void NameDictionary::Dump(JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int size = Size();
    for (int hashIndex = 0; hashIndex < size; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole()) {
            JSTaggedValue val(GetValue(hashIndex));
            os << std::right << std::setw(DUMP_PROPERTY_OFFSET);
            DumpPropertyKey(key, os);
            os << ": ";
            val.DumpTaggedValue(thread, os);
            os << " ";
            DumpAttr(GetAttributes(hashIndex), false, os);
            os << "\n";
        }
    }
}

void GlobalDictionary::Dump(JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int size = Size();
    for (int hashIndex = 0; hashIndex < size; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole()) {
            JSTaggedValue val(GetValue(hashIndex));
            os << std::right << std::setw(DUMP_PROPERTY_OFFSET);
            DumpPropertyKey(key, os);
            os << " : ";
            val.DumpTaggedValue(thread, os);
            os << " ";
            DumpAttr(GetAttributes(hashIndex), false, os);
            os << "\n";
        }
    }
}

void LayoutInfo::Dump([[maybe_unused]] JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int num = NumberOfElements();
    for (int i = 0; i < num; i++) {
        JSTaggedValue key = GetKey(i);
        PropertyAttributes attr = GetAttr(i);
        os << std::right << std::setw(DUMP_PROPERTY_OFFSET);
        os << "[" << i << "]: ";
        DumpPropertyKey(key, os);
        os << " : ";
        DumpAttr(attr, true, os);
        os << "\n";
    }
}

void TransitionsDictionary::Dump(JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int size = Size();
    for (int hashIndex = 0; hashIndex < size; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole()) {
            os << std::right << std::setw(DUMP_PROPERTY_OFFSET);
            DumpPropertyKey(key, os);
            os << " : ";
            GetValue(hashIndex).DumpTaggedValue(thread, os);
            os << " : ";
            GetAttributes(hashIndex).DumpTaggedValue(thread, os);
            os << "\n";
        }
    }
}

void LinkedHashSet::Dump(JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int capacity = NumberOfElements() + NumberOfDeletedElements();
    for (int hashIndex = 0; hashIndex < capacity; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole()) {
            os << std::right << std::setw(DUMP_PROPERTY_OFFSET);
            key.DumpTaggedValue(thread, os);
            os << "\n";
        }
    }
}

void LinkedHashMap::Dump(JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int capacity = NumberOfElements() + NumberOfDeletedElements();
    for (int hashIndex = 0; hashIndex < capacity; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole()) {
            JSTaggedValue val(GetValue(hashIndex));
            os << std::right << std::setw(DUMP_PROPERTY_OFFSET);
            key.DumpTaggedValue(thread, os);
            os << ": ";
            val.DumpTaggedValue(thread, os);
            os << "\n";
        }
    }
}

void JSObject::Dump(JSThread *thread, std::ostream &os) const
{
    DISALLOW_GARBAGE_COLLECTION;
    JSHClass *jshclass = GetJSHClass();
    os << " - hclass: " << std::hex << jshclass << "\n";
    os << " - prototype: ";
    jshclass->GetPrototype().DumpTaggedValue(thread, os);
    os << "\n";

    TaggedArray *elements = TaggedArray::Cast(GetElements().GetTaggedObject());
    os << " - elements: " << std::hex << elements;
    if (elements->GetLength() == 0) {
        os << " NONE\n";
    } else if (!elements->IsDictionaryMode()) {
        DumpArrayClass(thread, elements, os);
    } else {
        NumberDictionary *dict = NumberDictionary::Cast(elements);
        os << " <NumberDictionary[" << std::dec << dict->EntriesCount() << "]>\n";
        dict->Dump(thread, os);
    }

    TaggedArray *properties = TaggedArray::Cast(GetProperties().GetTaggedObject());
    os << " - properties: " << std::hex << properties;
    if (IsJSGlobalObject()) {
        GlobalDictionary *dict = GlobalDictionary::Cast(properties);
        os << " <GlobalDictionary[" << std::dec << dict->EntriesCount() << "]>\n";
        dict->Dump(thread, os);
        return;
    }

    if (!properties->IsDictionaryMode()) {
        JSTaggedValue attrs = jshclass->GetAttributes();
        if (attrs.IsNull()) {
            return;
        }

        LayoutInfo *layoutInfo = LayoutInfo::Cast(attrs.GetTaggedObject());
        int propNumber = jshclass->GetPropertiesNumber();
        os << " <LayoutInfo[" << std::dec << propNumber << "]>\n";
        for (int i = 0; i < propNumber; i++) {
            JSTaggedValue key = layoutInfo->GetKey(i);
            PropertyAttributes attr = layoutInfo->GetAttr(i);
            ASSERT(i == static_cast<int>(attr.GetOffset()));
            os << "     " << std::right << std::setw(DUMP_PROPERTY_OFFSET);
            DumpPropertyKey(key, os);
            os << ": (";
            JSTaggedValue val;
            if (attr.IsInlinedProps()) {
                val = GetPropertyInlinedProps(i);
            } else {
                val = properties->Get(i - JSHClass::DEFAULT_CAPACITY_OF_IN_OBJECTS);
            }
            val.DumpTaggedValue(thread, os);
            os << ") ";
            DumpAttr(attr, true, os);
            os << "\n";
        }
    } else {
        NameDictionary *dict = NameDictionary::Cast(properties);
        os << " <NameDictionary[" << std::dec << dict->EntriesCount() << "]>\n";
        dict->Dump(thread, os);
    }
}

void AccessorData::Dump(JSThread *thread, std::ostream &os) const
{
    auto *hclass = GetClass();
    if (hclass->GetObjectType() == JSType::INTERNAL_ACCESSOR) {
        os << " - Getter: " << reinterpret_cast<void *>(GetGetter().GetTaggedObject()) << "\n";
        os << " - Setter: " << reinterpret_cast<void *>(GetSetter().GetTaggedObject()) << "\n";
        return;
    }

    os << " - Getter: ";
    GetGetter().DumpTaggedValue(thread, os);
    os << "\n";

    os << " - Setter: ";
    GetSetter().DumpTaggedValue(thread, os);
    os << "\n";
}

void Program::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Location: ";
    GetLocation().Dump(thread);
    os << "\n";
    os << " - ConstantPool: ";
    GetConstantPool().Dump(thread);
    os << "\n";
    os << " - MainFunction: ";
    GetMainFunction().Dump(thread);
    os << "\n";
    os << " - MethodsData: " << GetMethodsData() << "\n";
    os << " - NumberMethods: " << GetNumberMethods() << "\n";
}

void ConstantPool::Dump(JSThread *thread, std::ostream &os) const
{
    DumpArrayClass(thread, this, os);
}

void JSFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - ProtoOrDynClass: ";
    GetProtoOrDynClass().Dump(thread);
    os << "\n";
    os << " - LexicalEnv: ";
    GetLexicalEnv().Dump(thread);
    os << "\n";
    os << " - HomeObject: ";
    GetHomeObject().Dump(thread);
    os << "\n";
    os << " - FunctionInfoFlag: ";
    GetFunctionInfoFlag().Dump(thread);
    os << "\n";
    os << " - FunctionExtraInfo: ";
    GetFunctionExtraInfo().Dump(thread);
    os << "\n";
    os << " - ConstantPool: ";
    GetConstantPool().Dump(thread);
    os << "\n";
    os << " - ProfileTypeInfo: ";
    GetProfileTypeInfo().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSHClass::Dump(JSThread *thread, std::ostream &os) const
{
    DumpHClass(thread, this, os, true);
}

void JSBoundFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - BoundTarget: ";
    GetBoundTarget().DumpTaggedValue(thread, os);
    os << "\n";

    os << " - BoundThis: ";
    GetBoundThis().DumpTaggedValue(thread, os);
    os << "\n";

    os << " - BoundArguments: ";
    GetBoundArguments().DumpTaggedValue(thread, os);
    os << "\n";

    JSObject::Dump(thread, os);
}

void JSPrimitiveRef::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - SubValue : ";
    GetValue().DumpTaggedValue(thread, os);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSDate::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - time: " << GetTime().GetDouble() << "\n";
    os << " - localOffset: " << GetLocalOffset().GetDouble() << "\n";
    JSObject::Dump(thread, os);
}

void JSMap::Dump(JSThread *thread, std::ostream &os) const
{
    LinkedHashMap *map = LinkedHashMap::Cast(GetLinkedMap().GetTaggedObject());
    os << " - elements: " << std::dec << map->NumberOfElements() << "\n";
    os << " - deleted-elements: " << std::dec << map->NumberOfDeletedElements() << "\n";
    os << " - capacity: " << std::dec << map->Capacity() << "\n";
    JSObject::Dump(thread, os);

    os << " <NameDictionary[" << map->NumberOfElements() << "]>\n";
    map->Dump(thread, os);
}

void JSForInIterator::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Object : ";
    GetObject().DumpTaggedValue(thread, os);
    os << "\n";
    os << " - WasVisited : ";
    GetWasVisited().DumpTaggedValue(thread, os);
    os << "\n";
    os << " - VisitedKeys : ";
    GetVisitedKeys().DumpTaggedValue(thread, os);
    os << "\n";
    os << " - RemainingKeys : ";
    GetRemainingKeys().DumpTaggedValue(thread, os);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSMapIterator::Dump(JSThread *thread, std::ostream &os) const
{
    LinkedHashMap *map = LinkedHashMap::Cast(GetIteratedMap().GetTaggedObject());
    os << " - elements: " << std::dec << map->NumberOfElements() << "\n";
    os << " - deleted-elements: " << std::dec << map->NumberOfDeletedElements() << "\n";
    os << " - capacity: " << std::dec << map->Capacity() << "\n";
    os << " - nextIndex: " << std::dec << GetNextIndex().GetInt() << "\n";
    os << " - IterationKind: " << std::dec << GetIterationKind().GetInt() << "\n";
    JSObject::Dump(thread, os);

    os << " <NameDictionary[" << map->NumberOfElements() << "]>\n";
    map->Dump(thread, os);
}

void JSSet::Dump(JSThread *thread, std::ostream &os) const
{
    LinkedHashSet *set = LinkedHashSet::Cast(GetLinkedSet().GetTaggedObject());
    os << " - elements: " << std::dec << set->NumberOfElements() << "\n";
    os << " - deleted-elements: " << std::dec << set->NumberOfDeletedElements() << "\n";
    os << " - capacity: " << std::dec << set->Capacity() << "\n";
    JSObject::Dump(thread, os);

    os << " <NameDictionary[" << set->NumberOfElements() << "]>\n";
    set->Dump(thread, os);
}

void JSWeakMap::Dump(JSThread *thread, std::ostream &os) const
{
    LinkedHashMap *map = LinkedHashMap::Cast(GetLinkedMap().GetTaggedObject());
    os << " - length: " << std::dec << GetSize() << "\n";
    os << " - elements: " << std::dec << map->NumberOfElements() << "\n";
    os << " - deleted-elements: " << std::dec << map->NumberOfDeletedElements() << "\n";
    os << " - capacity: " << std::dec << map->Capacity() << "\n";
    JSObject::Dump(thread, os);

    os << " <NameDictionary[" << map->NumberOfElements() << "]>\n";
    map->Dump(thread, os);
}

void JSWeakSet::Dump(JSThread *thread, std::ostream &os) const
{
    LinkedHashSet *set = LinkedHashSet::Cast(GetLinkedSet().GetTaggedObject());
    os << " - size: " << std::dec << GetSize() << "\n";
    os << " - elements: " << std::dec << set->NumberOfElements() << "\n";
    os << " - deleted-elements: " << std::dec << set->NumberOfDeletedElements() << "\n";
    os << " - capacity: " << std::dec << set->Capacity() << "\n";
    JSObject::Dump(thread, os);

    os << " <NameDictionary[" << set->NumberOfElements() << "]>\n";
    set->Dump(thread, os);
}

void JSSetIterator::Dump(JSThread *thread, std::ostream &os) const
{
    LinkedHashSet *set = LinkedHashSet::Cast(GetIteratedSet().GetTaggedObject());
    os << " - elements: " << std::dec << set->NumberOfElements() << "\n";
    os << " - deleted-elements: " << std::dec << set->NumberOfDeletedElements() << "\n";
    os << " - capacity: " << std::dec << set->Capacity() << "\n";
    os << " - nextIndex: " << std::dec << GetNextIndex().GetInt() << "\n";
    os << " - IterationKind: " << std::dec << GetIterationKind().GetInt() << "\n";
    JSObject::Dump(thread, os);

    os << " <NameDictionary[" << set->NumberOfElements() << "]>\n";
    set->Dump(thread, os);
}

void JSArray::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - length: " << std::dec << GetArrayLength() << "\n";
    JSObject::Dump(thread, os);
}

void JSArrayIterator::Dump(JSThread *thread, std::ostream &os) const
{
    JSArray *array = JSArray::Cast(GetIteratedArray().GetTaggedObject());
    os << " - length: " << std::dec << array->GetArrayLength() << "\n";
    os << " - nextIndex: " << std::dec << GetNextIndex().GetInt() << "\n";
    os << " - IterationKind: " << std::dec << GetIterationKind().GetInt() << "\n";
    JSObject::Dump(thread, os);
}

void JSStringIterator::Dump(JSThread *thread, std::ostream &os) const
{
    EcmaString *str = EcmaString::Cast(GetIteratedString().GetTaggedObject());
    os << " - IteratedString: " << str->GetCString().get() << "\n";
    os << " - StringIteratorNextIndex: " << std::dec << GetStringIteratorNextIndex().GetInt() << "\n";
    JSObject::Dump(thread, os);
}
void JSTypedArray::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - viewed-array-buffer: ";
    GetViewedArrayBuffer().Dump(thread);
    os << " - typed-array-name: ";
    GetTypedArrayName().Dump(thread);
    os << " - byte-length: ";
    GetByteLength().Dump(thread);
    os << " - byte-offset: ";
    GetByteOffset().Dump(thread);
    os << " - array-length: ";
    GetArrayLength().Dump(thread);
    JSObject::Dump(thread, os);
}

void JSRegExp::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - LastIndex: ";
    GetLastIndex().Dump(thread);
    os << "\n";
    os << " - ByteCodeBuffer: ";
    GetByteCodeBuffer().Dump(thread);
    os << "\n";
    os << " - OriginalSource: ";
    GetOriginalSource().Dump(thread);
    os << "\n";
    os << " - OriginalFlags: ";
    GetOriginalFlags().Dump(thread);
    os << "\n";
    os << " - Length: ";
    GetLength().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSProxy::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Target: ";
    os << "\n";
    JSObject::Cast(GetTarget().GetTaggedObject())->Dump(thread, os);
    os << " - Handler: ";
    os << "\n";
    JSObject::Cast(GetHandler().GetTaggedObject())->Dump(thread, os);
    os << "\n";
}

void JSSymbol::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - hash-field: ";
    JSTaggedValue hashField = GetHashField();
    hashField.Dump(thread);
    os << " - flags: ";
    JSTaggedValue flags = GetFlags();
    flags.Dump(thread);
    os << " - description: ";
    JSTaggedValue description = GetDescription();
    description.Dump(thread);
}

void LexicalEnv::Dump(JSThread *thread, std::ostream &os) const
{
    DumpArrayClass(thread, this, os);
}

// NOLINTNEXTLINE(readability-function-size)
void GlobalEnv::Dump(JSThread *thread, std::ostream &os) const
{
    auto globalConst = thread->GlobalConstants();
    os << " - ObjectFunction: ";
    GetObjectFunction().GetTaggedValue().Dump(thread, os);
    os << " - FunctionFunction: ";
    GetFunctionFunction().GetTaggedValue().Dump(thread, os);
    os << " - NumberFunction: ";
    GetNumberFunction().GetTaggedValue().Dump(thread, os);
    os << " - DateFunction: ";
    GetDateFunction().GetTaggedValue().Dump(thread, os);
    os << " - BooleanFunction: ";
    GetBooleanFunction().GetTaggedValue().Dump(thread, os);
    os << " - ErrorFunction: ";
    GetErrorFunction().GetTaggedValue().Dump(thread, os);
    os << " - ArrayFunction: ";
    GetArrayFunction().GetTaggedValue().Dump(thread, os);
    os << " - TypedArrayFunction: ";
    GetTypedArrayFunction().GetTaggedValue().Dump(thread, os);
    os << " - Int8ArrayFunction: ";
    GetInt8ArrayFunction().GetTaggedValue().Dump(thread, os);
    os << " - Uint8ArrayFunction: ";
    GetUint8ArrayFunction().GetTaggedValue().Dump(thread, os);
    os << " - Uint8ClampedArrayFunction: ";
    GetUint8ClampedArrayFunction().GetTaggedValue().Dump(thread, os);
    os << " - Int16ArrayFunction: ";
    GetInt16ArrayFunction().GetTaggedValue().Dump(thread, os);
    os << " - ArrayBufferFunction: ";
    GetArrayBufferFunction().GetTaggedValue().Dump(thread, os);
    os << " - SymbolFunction: ";
    GetSymbolFunction().GetTaggedValue().Dump(thread, os);
    os << " - RangeErrorFunction: ";
    GetRangeErrorFunction().GetTaggedValue().Dump(thread, os);
    os << " - ReferenceErrorFunction: ";
    GetReferenceErrorFunction().GetTaggedValue().Dump(thread, os);
    os << " - TypeErrorFunction: ";
    GetTypeErrorFunction().GetTaggedValue().Dump(thread, os);
    os << " - URIErrorFunction: ";
    GetURIErrorFunction().GetTaggedValue().Dump(thread, os);
    os << " - SyntaxErrorFunction: ";
    GetSyntaxErrorFunction().GetTaggedValue().Dump(thread, os);
    os << " - EvalErrorFunction: ";
    GetEvalErrorFunction().GetTaggedValue().Dump(thread, os);
    os << " - RegExpFunction: ";
    GetRegExpFunction().GetTaggedValue().Dump(thread, os);
    os << " - BuiltinsSetFunction: ";
    GetBuiltinsSetFunction().GetTaggedValue().Dump(thread, os);
    os << " - BuiltinsMapFunction: ";
    GetBuiltinsMapFunction().GetTaggedValue().Dump(thread, os);
    os << " - BuiltinsWeakSetFunction: ";
    GetBuiltinsWeakSetFunction().GetTaggedValue().Dump(thread, os);
    os << " - BuiltinsWeakMapFunction: ";
    GetBuiltinsWeakMapFunction().GetTaggedValue().Dump(thread, os);
    os << " - MathFunction: ";
    GetMathFunction().GetTaggedValue().Dump(thread, os);
    os << " - JsonFunction: ";
    GetJsonFunction().GetTaggedValue().Dump(thread, os);
    os << " - StringFunction: ";
    GetStringFunction().GetTaggedValue().Dump(thread, os);
    os << " - ProxyFunction: ";
    GetProxyFunction().GetTaggedValue().Dump(thread, os);
    os << " - ReflectFunction: ";
    GetReflectFunction().GetTaggedValue().Dump(thread, os);
    os << " - AsyncFunction: ";
    GetAsyncFunction().GetTaggedValue().Dump(thread, os);
    os << " - AsyncFunctionPrototype: ";
    GetAsyncFunctionPrototype().GetTaggedValue().Dump(thread, os);
    os << " - JSGlobalObject: ";
    GetJSGlobalObject().GetTaggedValue().Dump(thread, os);
    os << " - EmptyArray: ";
    GetEmptyArray().GetTaggedValue().Dump(thread, os);
    os << " - EmptyString ";
    globalConst->GetEmptyString().Dump(thread, os);
    os << " - EmptyTaggedQueue: ";
    GetEmptyTaggedQueue().GetTaggedValue().Dump(thread, os);
    os << " - PrototypeString: ";
    globalConst->GetPrototypeString().Dump(thread, os);
    os << " - HasInstanceSymbol: ";
    GetHasInstanceSymbol().GetTaggedValue().Dump(thread, os);
    os << " - IsConcatSpreadableSymbol: ";
    GetIsConcatSpreadableSymbol().GetTaggedValue().Dump(thread, os);
    os << " - ToStringTagSymbol: ";
    GetToStringTagSymbol().GetTaggedValue().Dump(thread, os);
    os << " - IteratorSymbol: ";
    GetIteratorSymbol().GetTaggedValue().Dump(thread, os);
    os << " - MatchSymbol: ";
    GetMatchSymbol().GetTaggedValue().Dump(thread, os);
    os << " - ReplaceSymbol: ";
    GetReplaceSymbol().GetTaggedValue().Dump(thread, os);
    os << " - SearchSymbol: ";
    GetSearchSymbol().GetTaggedValue().Dump(thread, os);
    os << " - SpeciesSymbol: ";
    GetSpeciesSymbol().GetTaggedValue().Dump(thread, os);
    os << " - SplitSymbol: ";
    GetSplitSymbol().GetTaggedValue().Dump(thread, os);
    os << " - ToPrimitiveSymbol: ";
    GetToPrimitiveSymbol().GetTaggedValue().Dump(thread, os);
    os << " - UnscopablesSymbol: ";
    GetUnscopablesSymbol().GetTaggedValue().Dump(thread, os);
    os << " - HoleySymbol: ";
    GetHoleySymbol().GetTaggedValue().Dump(thread, os);
    os << " - ConstructorString: ";
    globalConst->GetConstructorString().Dump(thread, os);
    os << " - IteratorPrototype: ";
    GetIteratorPrototype().GetTaggedValue().Dump(thread, os);
    os << " - ForinIteratorPrototype: ";
    GetForinIteratorPrototype().GetTaggedValue().Dump(thread, os);
    os << " - StringIterator: ";
    GetStringIterator().GetTaggedValue().Dump(thread, os);
    os << " - MapIteratorPrototype: ";
    GetMapIteratorPrototype().GetTaggedValue().Dump(thread, os);
    os << " - SetIteratorPrototype: ";
    GetSetIteratorPrototype().GetTaggedValue().Dump(thread, os);
    os << " - ArrayIteratorPrototype: ";
    GetArrayIteratorPrototype().GetTaggedValue().Dump(thread, os);
    os << " - StringIteratorPrototype: ";
    GetStringIteratorPrototype().GetTaggedValue().Dump(thread, os);
    os << " - LengthString: ";
    globalConst->GetLengthString().Dump(thread, os);
    os << " - ValueString: ";
    globalConst->GetValueString().Dump(thread, os);
    os << " - WritableString: ";
    globalConst->GetWritableString().Dump(thread, os);
    os << " - GetString: ";
    globalConst->GetGetString().Dump(thread, os);
    os << " - SetString: ";
    globalConst->GetSetString().Dump(thread, os);
    os << " - EnumerableString: ";
    globalConst->GetEnumerableString().Dump(thread, os);
    os << " - ConfigurableString: ";
    globalConst->GetConfigurableString().Dump(thread, os);
    os << " - NameString: ";
    globalConst->GetNameString().Dump(thread, os);
    os << " - ValueOfString: ";
    globalConst->GetValueOfString().Dump(thread, os);
    os << " - ToStringString: ";
    globalConst->GetToStringString().Dump(thread, os);
    os << " - ToLocaleStringString: ";
    globalConst->GetToLocaleStringString().Dump(thread, os);
    os << " - UndefinedString: ";
    globalConst->GetUndefinedString().Dump(thread, os);
    os << " - NullString: ";
    globalConst->GetNullString().Dump(thread, os);
    os << " - TrueString: ";
    globalConst->GetTrueString().Dump(thread, os);
    os << " - FalseString: ";
    globalConst->GetFalseString().Dump(thread, os);
    os << " - RegisterSymbols: ";
    GetRegisterSymbols().GetTaggedValue().Dump(thread, os);
    os << " - ThrowTypeError: ";
    GetThrowTypeError().GetTaggedValue().Dump(thread, os);
    os << " - GetPrototypeOfString: ";
    globalConst->GetGetPrototypeOfString().Dump(thread, os);
    os << " - SetPrototypeOfString: ";
    globalConst->GetSetPrototypeOfString().Dump(thread, os);
    os << " - IsExtensibleString: ";
    globalConst->GetIsExtensibleString().Dump(thread, os);
    os << " - PreventExtensionsString: ";
    globalConst->GetPreventExtensionsString().Dump(thread, os);
    os << " - GetOwnPropertyDescriptorString: ";
    globalConst->GetGetOwnPropertyDescriptorString().Dump(thread, os);
    os << " - DefinePropertyString: ";
    globalConst->GetDefinePropertyString().Dump(thread, os);
    os << " - HasString: ";
    globalConst->GetHasString().Dump(thread, os);
    os << " - DeletePropertyString: ";
    globalConst->GetDeletePropertyString().Dump(thread, os);
    os << " - EnumerateString: ";
    globalConst->GetEnumerateString().Dump(thread, os);
    os << " - OwnKeysString: ";
    globalConst->GetOwnKeysString().Dump(thread, os);
    os << " - ApplyString: ";
    globalConst->GetApplyString().Dump(thread, os);
    os << " - ProxyString: ";
    globalConst->GetProxyString().Dump(thread, os);
    os << " - RevokeString: ";
    globalConst->GetRevokeString().Dump(thread, os);
    os << " - ProxyConstructString: ";
    globalConst->GetProxyConstructString().Dump(thread, os);
    os << " - ProxyCallString: ";
    globalConst->GetProxyCallString().Dump(thread, os);
    os << " - DoneString: ";
    globalConst->GetDoneString().Dump(thread, os);
    os << " - NegativeZeroString: ";
    globalConst->GetNegativeZeroString().Dump(thread, os);
    os << " - NextString: ";
    globalConst->GetNextString().Dump(thread, os);
    os << " - PromiseThenString: ";
    globalConst->GetPromiseThenString().Dump(thread, os);
    os << " - PromiseFunction: ";
    GetPromiseFunction().GetTaggedValue().Dump(thread, os);
    os << " - PromiseReactionJob: ";
    GetPromiseReactionJob().GetTaggedValue().Dump(thread, os);
    os << " - PromiseResolveThenableJob: ";
    GetPromiseResolveThenableJob().GetTaggedValue().Dump(thread, os);
    os << " - ScriptJobString: ";
    globalConst->GetScriptJobString().Dump(thread, os);
    os << " - PromiseString: ";
    globalConst->GetPromiseString().Dump(thread, os);
    os << " - IdentityString: ";
    globalConst->GetIdentityString().Dump(thread, os);
    os << " - AsyncFunctionString: ";
    globalConst->GetAsyncFunctionString().Dump(thread, os);
    os << " - ThrowerString: ";
    globalConst->GetThrowerString().Dump(thread, os);
    os << " - Undefined: ";
    globalConst->GetUndefined().Dump(thread, os);
}

void JSDataView::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - data-view: ";
    GetDataView().Dump(thread);
    os << " - buffer: ";
    GetViewedArrayBuffer().Dump(thread);
    os << " - byte-length: ";
    GetByteLength().Dump(thread);
    os << " - byte-offset: ";
    GetByteOffset().Dump(thread);
}

void JSArrayBuffer::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - byte-length: ";
    GetArrayBufferByteLength().Dump(thread);
    os << " - buffer-data: ";
    GetArrayBufferData().Dump(thread);
    os << " - Shared: ";
    GetShared().Dump(thread);
}

void PromiseReaction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - promise-capability: ";
    GetPromiseCapability().Dump(thread);
    os << " - type: ";
    GetType().Dump(thread);
    os << " - handler: ";
    GetHandler().Dump(thread);
}

void PromiseCapability::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - promise: ";
    GetPromise().Dump(thread);
    os << " - resolve: ";
    GetResolve().Dump(thread);
    os << " - reject: ";
    GetReject().Dump(thread);
}

void PromiseIteratorRecord::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - iterator: ";
    GetIterator().Dump(thread);
    os << " - done: ";
    GetDone().Dump(thread);
}

void PromiseRecord::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - value: ";
    GetValue().Dump(thread);
}

void ResolvingFunctionsRecord::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - resolve-function: ";
    GetResolveFunction().Dump(thread);
    os << " - reject-function: ";
    GetRejectFunction().Dump(thread);
}

void JSPromise::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - promise-state: ";
    GetPromiseState().Dump(thread);
    os << " - promise-result: ";
    GetPromiseResult().Dump(thread);
    os << " - promise-fulfill-reactions: ";
    GetPromiseFulfillReactions().Dump(thread);
    os << " - promise-reject-reactions: ";
    GetPromiseRejectReactions().Dump(thread);
    os << " - promise-is-handled: ";
    GetPromiseIsHandled().Dump(thread);
    JSObject::Dump(thread, os);
}

void JSPromiseReactionsFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - promise: ";
    GetPromise().Dump(thread);
    os << " - already-resolved: ";
    GetAlreadyResolved().Dump(thread);
    JSObject::Dump(thread, os);
}

void JSPromiseExecutorFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - capability: ";
    GetCapability().Dump(thread);
    JSObject::Dump(thread, os);
}

void JSPromiseAllResolveElementFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - index: ";
    GetIndex().Dump(thread);
    os << " - values: ";
    GetValues().Dump(thread);
    os << " - capability: ";
    GetCapabilities().Dump(thread);
    os << " - remaining-elements: ";
    GetRemainingElements().Dump(thread);
    os << " - already-called: ";
    GetAlreadyCalled().Dump(thread);
    JSObject::Dump(thread, os);
}

void MicroJobQueue::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - promise-job-queue: ";
    GetPromiseJobQueue().Dump(thread);
    os << " - script-job-queue: ";
    GetScriptJobQueue().Dump(thread);
}

void PendingJob::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - job: ";
    GetJob().Dump(thread);
    os << " - arguments: ";
    GetArguments().Dump(thread);
}

void CompletionRecord::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - type: ";
    GetType().Dump(thread);
    os << " - value: ";
    GetValue().Dump(thread);
}

void JSProxyRevocFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - RevocableProxy: ";
    os << "\n";
    GetRevocableProxy().Dump(thread);
    os << "\n";
}

void JSAsyncFunction::Dump(JSThread *thread, std::ostream &os) const
{
    JSFunction::Dump(thread, os);
}

void JSAsyncAwaitStatusFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - AsyncContext: ";
    os << "\n";
    GetAsyncContext().Dump(thread);
    os << "\n";
}

void JSGeneratorFunction::Dump(JSThread *thread, std::ostream &os) const
{
    JSFunction::Dump(thread, os);
}

void JSIntlBoundFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - NumberFormat: ";
    GetNumberFormat().Dump(thread);
    os << "\n";
    os << " - DateTimeFormat: ";
    GetDateTimeFormat().Dump(thread);
    os << "\n";
    os << " - Collator: ";
    GetCollator().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void PropertyBox::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Value: ";
    GetValue().Dump(thread);
    os << "\n";
}

void PrototypeHandler::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - HandlerInfo: ";
    GetHandlerInfo().Dump(thread);
    os << "\n";
    os << " - ProtoCell: ";
    GetHandlerInfo().Dump(thread);
    os << "\n";
    os << " - Holder: ";
    GetHandlerInfo().Dump(thread);
    os << "\n";
}

void TransitionHandler::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - HandlerInfo: ";
    GetHandlerInfo().Dump(thread);
    os << "\n";
    os << " - TransitionHClass: ";
    GetTransitionHClass().Dump(thread);
    os << "\n";
}

void JSRealm::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Value: ";
    GetValue().Dump(thread);
    os << "\n";
    os << " - GlobalEnv: ";
    GetGlobalEnv().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSIntl::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - FallbackSymbol: ";
    GetFallbackSymbol().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSLocale::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - IcuField: ";
    GetIcuField().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSDateTimeFormat::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Locale: ";
    GetLocale().Dump(thread);
    os << "\n";
    os << " - Calendar: ";
    GetCalendar().Dump(thread);
    os << "\n";
    os << " - NumberingSystem: ";
    GetNumberingSystem().Dump(thread);
    os << "\n";
    os << " - TimeZone: ";
    GetTimeZone().Dump(thread);
    os << "\n";
    os << " - HourCycle: ";
    GetHourCycle().Dump(thread);
    os << "\n";
    os << " - LocaleIcu: ";
    GetLocaleIcu().Dump(thread);
    os << "\n";
    os << " - SimpleDateTimeFormatIcu: ";
    GetSimpleDateTimeFormatIcu().Dump(thread);
    os << "\n";
    os << " - Iso8601: ";
    GetIso8601().Dump(thread);
    os << "\n";
    os << " - DateStyle: ";
    GetDateStyle().Dump(thread);
    os << "\n";
    os << " - TimeStyle: ";
    GetTimeStyle().Dump(thread);
    os << "\n";
    os << " - BoundFormat: ";
    GetBoundFormat().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSRelativeTimeFormat::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Locale: ";
    GetLocale().Dump(thread);
    os << "\n";
    os << " - InitializedRelativeTimeFormat: ";
    GetInitializedRelativeTimeFormat().Dump(thread);
    os << "\n";
    os << " - NumberingSystem: ";
    GetNumberingSystem().Dump(thread);
    os << "\n";
    os << " - Style: ";
    GetStyle().Dump(thread);
    os << "\n";
    os << " - Numeric: ";
    GetNumeric().Dump(thread);
    os << "\n";
    os << " - AvailableLocales: ";
    GetAvailableLocales().Dump(thread);
    os << "\n";
    os << " - IcuField: ";
    GetIcuField().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSNumberFormat::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Locale: ";
    GetLocale().Dump(thread);
    os << "\n" << " - NumberingSystem: ";
    GetNumberingSystem().Dump(thread);
    os << "\n" << " - Style: ";
    GetStyle().Dump(thread);
    os << "\n" << " - Currency: ";
    GetCurrency().Dump(thread);
    os << "\n" << " - CurrencyDisplay: ";
    GetCurrencyDisplay().Dump(thread);
    os << "\n" << " - CurrencySign: ";
    GetCurrencySign().Dump(thread);
    os << "\n" << " - Unit: ";
    GetUnit().Dump(thread);
    os << "\n" << " - UnitDisplay: ";
    GetUnitDisplay().Dump(thread);
    os << "\n" << " - MinimumIntegerDigits: ";
    GetMinimumIntegerDigits().Dump(thread);
    os << "\n" << " - MinimumFractionDigits: ";
    GetMinimumFractionDigits().Dump(thread);
    os << "\n" << " - MaximumFractionDigits: ";
    GetMaximumFractionDigits().Dump(thread);
    os << "\n" << " - MinimumSignificantDigits: ";
    GetMinimumSignificantDigits().Dump(thread);
    os << "\n" << " - MaximumSignificantDigits: ";
    GetMaximumSignificantDigits().Dump(thread);
    os << "\n" << " - UseGrouping: ";
    GetUseGrouping().Dump(thread);
    os << "\n" << " - RoundingType: ";
    GetUseGrouping().Dump(thread);
    os << "\n" << " - Notation: ";
    GetMinimumIntegerDigits().Dump(thread);
    os << "\n" << " - CompactDisplay: ";
    GetMaximumSignificantDigits().Dump(thread);
    os << "\n" << " - SignDisplay: ";
    GetMinimumFractionDigits().Dump(thread);
    os << "\n" << " - BoundFormat: ";
    GetMaximumFractionDigits().Dump(thread);
    os << "\n" << " - IcuField: ";
    GetMinimumSignificantDigits().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSCollator::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - IcuField: ";
    GetIcuField().Dump(thread);
    os << "\n";
    os << " - Locale: ";
    GetLocale().Dump(thread);
    os << "\n";
    os << " - Usage: ";
    GetUsage().Dump(thread);
    os << "\n";
    os << " - Sensitivity: ";
    GetSensitivity().Dump(thread);
    os << "\n";
    os << " - IgnorePunctuation: ";
    GetIgnorePunctuation().Dump(thread);
    os << "\n";
    os << " - Collation: ";
    GetCollation().Dump(thread);
    os << "\n";
    os << " - Numeric: ";
    GetNumeric().Dump(thread);
    os << "\n";
    os << " - CaseFirst: ";
    GetCaseFirst().Dump(thread);
    os << "\n";
    os << " - BoundCompare: ";
    GetBoundCompare().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSPluralRules::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Locale: ";
    GetLocale().Dump(thread);
    os << "\n";
    os << " - InitializedPluralRules: ";
    GetInitializedPluralRules().Dump(thread);
    os << "\n";
    os << " - Type: ";
    GetType().Dump(thread);
    os << "\n";
    os << " - MinimumIntegerDigits: ";
    GetMinimumIntegerDigits().Dump(thread);
    os << "\n";
    os << " - MinimumFractionDigits: ";
    GetMinimumFractionDigits().Dump(thread);
    os << "\n";
    os << " - MaximumFractionDigits: ";
    GetMaximumFractionDigits().Dump(thread);
    os << "\n";
    os << " - MinimumSignificantDigits: ";
    GetMinimumSignificantDigits().Dump(thread);
    os << "\n";
    os << " - MaximumSignificantDigits: ";
    GetMaximumSignificantDigits().Dump(thread);
    os << "\n";
    os << " - RoundingType: ";
    GetRoundingType().Dump(thread);
    os << "\n";
    os << " - IcuPR: ";
    GetIcuPR().Dump(thread);
    os << "\n";
    os << " - IcuNF: ";
    GetIcuNF().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSGeneratorObject::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - GeneratorState: ";
    GetGeneratorState().Dump(thread);
    os << "\n";
    os << " - GeneratorContext: ";
    GetGeneratorContext().Dump(thread);
    os << "\n";
    os << " - ResumeResult: ";
    GetResumeResult().Dump(thread);
    os << "\n";
    os << " - ResumeMode: ";
    GetResumeMode().Dump(thread);
    os << "\n";
    JSObject::Dump(thread, os);
}

void JSAsyncFuncObject::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Promise: ";
    GetPromise().Dump(thread);
    os << "\n";
}

void GeneratorContext::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - RegsArray: ";
    GetRegsArray().Dump(thread);
    os << "\n";
    os << " - Method: ";
    GetMethod().Dump(thread);
    os << "\n";
    os << " - Acc: ";
    GetAcc().Dump(thread);
    os << "\n";
    os << " - NRegs: ";
    GetNRegs().Dump(thread);
    os << "\n";
    os << " - BCOffset: ";
    GetBCOffset().Dump(thread);
    os << "\n";
    os << " - GeneratorObject: ";
    GetGeneratorObject().Dump(thread);
    os << "\n";
    os << " - LexicalEnv: ";
    GetLexicalEnv().Dump(thread);
    os << "\n";
}

void ProtoChangeMarker::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - HasChanged: " << GetHasChanged() << "\n";
}

void ProtoChangeDetails::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - ChangeListener: ";
    GetChangeListener().Dump(thread);
    os << "\n";
    os << " - RegisterIndex: ";
    GetRegisterIndex().Dump(thread);
    os << "\n";
}

void LexicalFunction::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Name: ";
    GetName().Dump(thread);
    os << "\n";
    os << " - NumberVRegs: ";
    GetNumberVRegs().Dump(thread);
    os << "\n";
    os << " - NumberICSlots: ";
    GetNumberICSlots().Dump(thread);
    os << "\n";
    os << " - Bytecode: ";
    GetBytecode().Dump(thread);
    os << "\n";
    os << " - Program: ";
    GetProgram().Dump(thread);
    os << "\n";
}

void JSFunctionExtraInfo::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - Callback: ";
    GetCallback().Dump(thread);
    os << "\n";
    os << " - Data: ";
    GetData().Dump(thread);
    os << "\n";
}

void MachineCode::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - InstructionSizeInBytes: ";
    GetInstructionSizeInBytes().Dump(thread);
    os << "\n";
}

void EcmaModule::Dump(JSThread *thread, std::ostream &os) const
{
    os << " - NameDictionary: ";
    GetNameDictionary().Dump(thread);
    os << "\n";
}

// ########################################################################################
// Dump for Snapshot
// ########################################################################################
static void DumpArrayClass([[maybe_unused]] JSThread *thread, const TaggedArray *arr,
                           std::vector<std::pair<CString, JSTaggedValue>> &vec)
{
    DISALLOW_GARBAGE_COLLECTION;
    array_size_t len = arr->GetLength();
    for (array_size_t i = 0; i < len; i++) {
        JSTaggedValue val(arr->Get(i));
        CString str = ToCString(i);
        vec.push_back(std::make_pair(str, val));
    }
}

static void DumpStringClass([[maybe_unused]] JSThread *thread, const EcmaString *str,
                            std::vector<std::pair<CString, JSTaggedValue>> &vec)
{
    vec.push_back(std::make_pair("string", JSTaggedValue(str)));
}

static void DumpDynClass([[maybe_unused]] JSThread *thread, TaggedObject *obj,
                         std::vector<std::pair<CString, JSTaggedValue>> &vec)
{
    JSHClass *jshclass = obj->GetClass();
    vec.push_back(std::make_pair("__proto__", jshclass->GetPrototype()));
}

static void DumpObject(JSThread *thread, TaggedObject *obj,
                       std::vector<std::pair<CString, JSTaggedValue>> &vec, bool isVmMode)
{
    DISALLOW_GARBAGE_COLLECTION;
    auto jsHclass = obj->GetClass();
    JSType type = jsHclass->GetObjectType();

    switch (type) {
        case JSType::HCLASS:
            DumpDynClass(thread, obj, vec);
            return;
        case JSType::TAGGED_ARRAY:
        case JSType::TAGGED_DICTIONARY:
            DumpArrayClass(thread, TaggedArray::Cast(obj), vec);
            return;
        case JSType::STRING:
            DumpStringClass(thread, EcmaString::Cast(obj), vec);
            return;
        case JSType::JS_NATIVE_POINTER:
            return;
        case JSType::JS_OBJECT:
        case JSType::JS_ERROR:
        case JSType::JS_EVAL_ERROR:
        case JSType::JS_RANGE_ERROR:
        case JSType::JS_TYPE_ERROR:
        case JSType::JS_REFERENCE_ERROR:
        case JSType::JS_URI_ERROR:
        case JSType::JS_SYNTAX_ERROR:
        case JSType::JS_ARGUMENTS:
        case JSType::JS_GLOBAL_OBJECT:
            JSObject::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_FUNCTION_BASE:
        case JSType::JS_FUNCTION:
            JSFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_BOUND_FUNCTION:
            JSBoundFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_SET:
            JSSet::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_MAP:
            JSMap::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_WEAK_SET:
            JSWeakSet::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_WEAK_MAP:
            JSWeakMap::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_REG_EXP:
            JSRegExp::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_DATE:
            JSDate::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_ARRAY:
            JSArray::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_TYPED_ARRAY:
        case JSType::JS_INT8_ARRAY:
        case JSType::JS_UINT8_ARRAY:
        case JSType::JS_UINT8_CLAMPED_ARRAY:
        case JSType::JS_INT16_ARRAY:
        case JSType::JS_UINT16_ARRAY:
        case JSType::JS_INT32_ARRAY:
        case JSType::JS_UINT32_ARRAY:
        case JSType::JS_FLOAT32_ARRAY:
        case JSType::JS_FLOAT64_ARRAY:
            JSTypedArray::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PROXY:
            JSProxy::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PRIMITIVE_REF:
            JSPrimitiveRef::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::SYMBOL:
            JSSymbol::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::ACCESSOR_DATA:
        case JSType::INTERNAL_ACCESSOR:
            AccessorData::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_DATA_VIEW:
            JSDataView::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::PROMISE_REACTIONS:
            PromiseReaction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::PROMISE_CAPABILITY:
            PromiseCapability::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::PROMISE_ITERATOR_RECORD:
            PromiseIteratorRecord::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::PROMISE_RECORD:
            PromiseRecord::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::RESOLVING_FUNCTIONS_RECORD:
            ResolvingFunctionsRecord::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PROMISE:
            JSPromise::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PROMISE_REACTIONS_FUNCTION:
            JSPromiseReactionsFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PROMISE_EXECUTOR_FUNCTION:
            JSPromiseExecutorFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PROMISE_ALL_RESOLVE_ELEMENT_FUNCTION:
            JSPromiseAllResolveElementFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::MICRO_JOB_QUEUE:
            MicroJobQueue::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::PENDING_JOB:
            PendingJob::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::COMPLETION_RECORD:
            CompletionRecord::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_ITERATOR:
        case JSType::JS_FORIN_ITERATOR:
        case JSType::JS_MAP_ITERATOR:
        case JSType::JS_SET_ITERATOR:
        case JSType::JS_ARRAY_ITERATOR:
        case JSType::JS_STRING_ITERATOR:
        case JSType::JS_ARRAY_BUFFER:
            JSArrayBuffer::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PROXY_REVOC_FUNCTION:
            JSProxyRevocFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_ASYNC_FUNCTION:
            JSAsyncFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_ASYNC_AWAIT_STATUS_FUNCTION:
            JSAsyncAwaitStatusFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_GENERATOR_FUNCTION:
            JSGeneratorFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_INTL_BOUND_FUNCTION:
            JSIntlBoundFunction::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_REALM:
            JSRealm::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_INTL:
            JSIntl::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_LOCALE:
            JSLocale::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_DATE_TIME_FORMAT:
            JSDateTimeFormat::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_RELATIVE_TIME_FORMAT:
            JSRelativeTimeFormat::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_NUMBER_FORMAT:
            JSNumberFormat::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_COLLATOR:
            JSCollator::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_PLURAL_RULES:
            JSPluralRules::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_GENERATOR_OBJECT:
            JSGeneratorObject::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_ASYNC_FUNC_OBJECT:
            JSAsyncFuncObject::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::JS_GENERATOR_CONTEXT:
            GeneratorContext::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::FUNCTION_EXTRA_INFO:
            JSFunctionExtraInfo::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        case JSType::ECMA_MODULE:
            EcmaModule::Cast(obj)->DumpForSnapshot(thread, vec);
            return;
        default:
            break;
    }
    if (isVmMode) {
        switch (type) {
            case JSType::PROPERTY_BOX:
                PropertyBox::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::TEMPLATE_MAP:
                DumpArrayClass(thread, TaggedArray::Cast(obj), vec);
                return;
            case JSType::LEXICAL_FUNCTION:
                LexicalFunction::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::GLOBAL_ENV:
                GlobalEnv::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::PROTO_CHANGE_MARKER:
                ProtoChangeMarker::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::PROTOTYPE_INFO:
                ProtoChangeDetails::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::PROGRAM:
                Program::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::MACHINE_CODE_OBJECT:
                MachineCode::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::TRANSITION_HANDLER:
                TransitionHandler::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            case JSType::PROTOTYPE_HANDLER:
                PrototypeHandler::Cast(obj)->DumpForSnapshot(thread, vec);
                return;
            default:
                UNREACHABLE();
                break;
        }
    }
}

static inline void EcmaStringToStd(CString &res, EcmaString *str)
{
    if (str->GetLength() == 0) {
        CString emptyStr = "EmptyString";
        res.append(emptyStr);
    }

    CString string = ConvertToString(str);
    res.append(string);
}

static void KeyToStd(CString &res, JSTaggedValue key)
{
    if (key.IsInt()) {
        res = std::to_string(key.GetInt());
    } else if (key.IsDouble()) {
        res = std::to_string(key.GetDouble());
    } else if (key.IsBoolean()) {
        res = key.IsTrue() ? "true" : "false";
    } else if (key.IsHeapObject()) {
        if (key.IsWeak()) {
            key.RemoveWeakTag();
        }
        if (key.IsString()) {
            EcmaStringToStd(res, EcmaString::Cast(key.GetTaggedObject()));
        } else if (key.IsSymbol()) {
            JSSymbol *sym = JSSymbol::Cast(key.GetTaggedObject());
            EcmaStringToStd(res, EcmaString::Cast(sym->GetDescription().GetTaggedObject()));
        }
    }
}

void JSTaggedValue::DumpForSnapshot(JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec, bool isVmMode) const
{
    if (IsHeapObject()) {
        return DumpObject(thread, GetTaggedObject(), vec, isVmMode);
    }

    UNREACHABLE();
}

void NumberDictionary::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                       std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int size = Size();
    for (int hashIndex = 0; hashIndex < size; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole() && !key.IsNull()) {
            JSTaggedValue val(GetValue(hashIndex));
            CString str = ToCString(static_cast<uint32_t>(JSTaggedNumber(key).GetNumber()));
            vec.push_back(std::make_pair(str, val));
        }
    }
}

void NameDictionary::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                     std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int size = Size();
    for (int hashIndex = 0; hashIndex < size; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole() && !key.IsNull()) {
            JSTaggedValue val(GetValue(hashIndex));
            CString str;
            KeyToStd(str, key);
            vec.push_back(std::make_pair(str, val));
        }
    }
}

void GlobalDictionary::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                       std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int size = Size();
    for (int hashIndex = 0; hashIndex < size; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole() && !key.IsNull()) {
            CString str;
            KeyToStd(str, key);
            JSTaggedValue val = GetValue(hashIndex);
            vec.push_back(std::make_pair(str, val));
        }
    }
}

void LinkedHashSet::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int capacity = NumberOfElements() + NumberOfDeletedElements();
    for (int hashIndex = 0; hashIndex < capacity; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole() && !key.IsNull()) {
            CString str;
            KeyToStd(str, key);
            vec.push_back(std::make_pair(str, JSTaggedValue::Hole()));
        }
    }
}

void LinkedHashMap::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DISALLOW_GARBAGE_COLLECTION;
    int capacity = NumberOfElements() + NumberOfDeletedElements();
    for (int hashIndex = 0; hashIndex < capacity; hashIndex++) {
        JSTaggedValue key(GetKey(hashIndex));
        if (!key.IsUndefined() && !key.IsHole() && !key.IsNull()) {
            JSTaggedValue val = GetValue(hashIndex);
            CString str;
            KeyToStd(str, key);
            vec.push_back(std::make_pair(str, val));
        }
    }
}

void JSObject::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DISALLOW_GARBAGE_COLLECTION;
    JSHClass *jshclass = GetJSHClass();
    vec.push_back(std::make_pair("__proto__", jshclass->GetPrototype()));

    TaggedArray *elements = TaggedArray::Cast(GetElements().GetTaggedObject());
    if (elements->GetLength() == 0) {
    } else if (!elements->IsDictionaryMode()) {
        DumpArrayClass(thread, elements, vec);
    } else {
        NumberDictionary *dict = NumberDictionary::Cast(elements);
        dict->DumpForSnapshot(thread, vec);
    }

    TaggedArray *properties = TaggedArray::Cast(GetProperties().GetTaggedObject());
    if (IsJSGlobalObject()) {
        GlobalDictionary *dict = GlobalDictionary::Cast(properties);
        dict->DumpForSnapshot(thread, vec);
        return;
    }

    if (!properties->IsDictionaryMode()) {
        JSTaggedValue attrs = jshclass->GetAttributes();
        if (attrs.IsNull()) {
            return;
        }

        LayoutInfo *layoutInfo = LayoutInfo::Cast(attrs.GetTaggedObject());
        int propNumber = jshclass->GetPropertiesNumber();
        for (int i = 0; i < propNumber; i++) {
            JSTaggedValue key = layoutInfo->GetKey(i);
            PropertyAttributes attr = layoutInfo->GetAttr(i);
            ASSERT(i == static_cast<int>(attr.GetOffset()));
            JSTaggedValue val;
            if (attr.IsInlinedProps()) {
                val = GetPropertyInlinedProps(i);
            } else {
                val = properties->Get(i - JSHClass::DEFAULT_CAPACITY_OF_IN_OBJECTS);
            }

            CString str;
            KeyToStd(str, key);
            vec.push_back(std::make_pair(str, val));
        }
    } else {
        NameDictionary *dict = NameDictionary::Cast(properties);
        dict->DumpForSnapshot(thread, vec);
    }
}

void JSHClass::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                               [[maybe_unused]] std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
}

void JSFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                 std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("ProtoOrDynClass"), GetProtoOrDynClass()));
    vec.push_back(std::make_pair(CString("LexicalEnv"), GetLexicalEnv()));
    vec.push_back(std::make_pair(CString("HomeObject"), GetHomeObject()));
    vec.push_back(std::make_pair(CString("FunctionInfoFlag"), GetFunctionInfoFlag()));
    vec.push_back(std::make_pair(CString("FunctionExtraInfo"), GetFunctionExtraInfo()));
    vec.push_back(std::make_pair(CString("ConstantPool"), GetConstantPool()));
    vec.push_back(std::make_pair(CString("ProfileTypeInfo"), GetProfileTypeInfo()));
    JSObject::DumpForSnapshot(thread, vec);
}

void Program::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                              std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Location"), GetLocation()));
    vec.push_back(std::make_pair(CString("ConstantPool"), GetConstantPool()));
    vec.push_back(std::make_pair(CString("MainFunction"), GetMainFunction()));
    // MethodsData is another native field, and we don't dump it for JS heap.
    vec.push_back(std::make_pair(CString("NumberMethods"), JSTaggedValue(GetNumberMethods())));
}

void ConstantPool::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                   std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DumpArrayClass(thread, this, vec);
}

void JSBoundFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                      std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    JSObject::DumpForSnapshot(thread, vec);

    vec.push_back(std::make_pair(CString("BoundTarget"), GetBoundTarget()));
    vec.push_back(std::make_pair(CString("BoundThis"), GetBoundThis()));
    vec.push_back(std::make_pair(CString("BoundArguments"), GetBoundArguments()));
}

void JSPrimitiveRef::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                     std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("subValue"), GetValue()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSDate::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                             std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("time"), GetTime()));
    vec.push_back(std::make_pair(CString("localOffset"), GetLocalOffset()));

    JSObject::DumpForSnapshot(thread, vec);
}

void JSMap::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                            std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    LinkedHashMap *map = LinkedHashMap::Cast(GetLinkedMap().GetTaggedObject());
    map->DumpForSnapshot(thread, vec);

    JSObject::DumpForSnapshot(thread, vec);
}

void JSForInIterator::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                      std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Object"), GetObject()));
    vec.push_back(std::make_pair(CString("WasVisited"), GetWasVisited()));
    vec.push_back(std::make_pair(CString("VisitedKeys"), GetVisitedKeys()));
    vec.push_back(std::make_pair(CString("RemainingKeys"), GetRemainingKeys()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSMapIterator::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    LinkedHashMap *map = LinkedHashMap::Cast(GetIteratedMap().GetTaggedObject());
    map->DumpForSnapshot(thread, vec);
    vec.push_back(std::make_pair(CString("NextIndex"), GetNextIndex()));
    vec.push_back(std::make_pair(CString("IterationKind"), GetIterationKind()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSSet::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                            std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    LinkedHashSet *set = LinkedHashSet::Cast(GetLinkedSet().GetTaggedObject());
    set->DumpForSnapshot(thread, vec);

    JSObject::DumpForSnapshot(thread, vec);
}

void JSWeakMap::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    LinkedHashMap *map = LinkedHashMap::Cast(GetLinkedMap().GetTaggedObject());
    map->DumpForSnapshot(thread, vec);

    JSObject::DumpForSnapshot(thread, vec);
}

void JSWeakSet::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    LinkedHashSet *set = LinkedHashSet::Cast(GetLinkedSet().GetTaggedObject());
    set->DumpForSnapshot(thread, vec);

    JSObject::DumpForSnapshot(thread, vec);
}
void JSSetIterator::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    LinkedHashSet *set = LinkedHashSet::Cast(GetIteratedSet().GetTaggedObject());
    set->DumpForSnapshot(thread, vec);
    vec.push_back(std::make_pair(CString("NextIndex"), GetNextIndex()));
    vec.push_back(std::make_pair(CString("IterationKind"), GetIterationKind()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSArray::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                              std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    JSObject::DumpForSnapshot(thread, vec);
}

void JSArrayIterator::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                      std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    JSArray *array = JSArray::Cast(GetIteratedArray().GetTaggedObject());
    array->DumpForSnapshot(thread, vec);
    vec.push_back(std::make_pair(CString("NextIndex"), GetNextIndex()));
    vec.push_back(std::make_pair(CString("IterationKind"), GetIterationKind()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSStringIterator::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                       std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("IteratedString"), GetIteratedString()));
    vec.push_back(std::make_pair(CString("StringIteratorNextIndex"), GetStringIteratorNextIndex()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSTypedArray::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                   std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("viewed-array-buffer"), GetViewedArrayBuffer()));
    vec.push_back(std::make_pair(CString("typed-array-name"), GetTypedArrayName()));
    vec.push_back(std::make_pair(CString("byte-length"), GetByteLength()));
    vec.push_back(std::make_pair(CString("byte-offset"), GetByteOffset()));
    vec.push_back(std::make_pair(CString("array-length"), GetArrayLength()));
}

void JSRegExp::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                               std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("originalSource"), GetOriginalSource()));
    vec.push_back(std::make_pair(CString("originalFlags"), GetOriginalFlags()));

    JSObject::DumpForSnapshot(thread, vec);
}

void JSProxy::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                              std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("target"), GetTarget()));
    vec.push_back(std::make_pair(CString("handler"), GetHandler()));
}

void JSSymbol::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                               std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("hash-field"), GetHashField()));
    vec.push_back(std::make_pair(CString("flags"), GetFlags()));
    vec.push_back(std::make_pair(CString("description"), GetDescription()));
}

void AccessorData::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                   std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("getter"), GetGetter()));
    vec.push_back(std::make_pair(CString("setter"), GetSetter()));
}

void LexicalEnv::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                 std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    DumpArrayClass(thread, this, vec);
}

void GlobalEnv::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    auto globalConst = thread->GlobalConstants();
    vec.push_back(std::make_pair(CString("ObjectFunction"), GetObjectFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("FunctionFunction"), GetFunctionFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("NumberFunction"), GetNumberFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("DateFunction"), GetDateFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("BooleanFunction"), GetBooleanFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ErrorFunction"), GetErrorFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ArrayFunction"), GetArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("TypedArrayFunction"), GetTypedArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Int8ArrayFunction"), GetInt8ArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Uint8ArrayFunction"), GetUint8ArrayFunction().GetTaggedValue()));
    vec.push_back(
        std::make_pair(CString("Uint8ClampedArrayFunction"), GetUint8ClampedArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Int16ArrayFunction"), GetInt16ArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Uint16ArrayFunction"), GetUint16ArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Int32ArrayFunction"), GetInt32ArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Uint32ArrayFunction"), GetUint32ArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Float32ArrayFunction"), GetFloat32ArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("Float64ArrayFunction"), GetFloat64ArrayFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ArrayBufferFunction"), GetArrayBufferFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("SymbolFunction"), GetSymbolFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("RangeErrorFunction"), GetRangeErrorFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ReferenceErrorFunction"), GetReferenceErrorFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("TypeErrorFunction"), GetTypeErrorFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("URIErrorFunction"), GetURIErrorFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("SyntaxErrorFunction"), GetSyntaxErrorFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("EvalErrorFunction"), GetEvalErrorFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("RegExpFunction"), GetRegExpFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("BuiltinsSetFunction"), GetBuiltinsSetFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("BuiltinsMapFunction"), GetBuiltinsMapFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("BuiltinsWeakSetFunction"), GetBuiltinsWeakSetFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("BuiltinsWeakMapFunction"), GetBuiltinsWeakMapFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("MathFunction"), GetMathFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("JsonFunction"), GetJsonFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("StringFunction"), GetStringFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ProxyFunction"), GetProxyFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ReflectFunction"), GetReflectFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("AsyncFunction"), GetAsyncFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("AsyncFunctionPrototype"), GetAsyncFunctionPrototype().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("JSGlobalObject"), GetJSGlobalObject().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("EmptyArray"), GetEmptyArray().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("EmptyString"), globalConst->GetEmptyString()));
    vec.push_back(std::make_pair(CString("EmptyTaggedQueue"), GetEmptyTaggedQueue().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("PrototypeString"), globalConst->GetPrototypeString()));
    vec.push_back(std::make_pair(CString("HasInstanceSymbol"), GetHasInstanceSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("IsConcatSpreadableSymbol"), GetIsConcatSpreadableSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ToStringTagSymbol"), GetToStringTagSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("IteratorSymbol"), GetIteratorSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("MatchSymbol"), GetMatchSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ReplaceSymbol"), GetReplaceSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("SearchSymbol"), GetSearchSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("SpeciesSymbol"), GetSpeciesSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("SplitSymbol"), GetSplitSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ToPrimitiveSymbol"), GetToPrimitiveSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("UnscopablesSymbol"), GetUnscopablesSymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("HoleySymbol"), GetHoleySymbol().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ConstructorString"), globalConst->GetConstructorString()));
    vec.push_back(std::make_pair(CString("IteratorPrototype"), GetIteratorPrototype().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ForinIteratorPrototype"), GetForinIteratorPrototype().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("StringIterator"), GetStringIterator().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("MapIteratorPrototype"), GetMapIteratorPrototype().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("SetIteratorPrototype"), GetSetIteratorPrototype().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ArrayIteratorPrototype"), GetArrayIteratorPrototype().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("StringIteratorPrototype"), GetStringIteratorPrototype().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("LengthString"), globalConst->GetLengthString()));
    vec.push_back(std::make_pair(CString("ValueString"), globalConst->GetValueString()));
    vec.push_back(std::make_pair(CString("WritableString"), globalConst->GetWritableString()));
    vec.push_back(std::make_pair(CString("GetString"), globalConst->GetGetString()));
    vec.push_back(std::make_pair(CString("SetString"), globalConst->GetSetString()));
    vec.push_back(std::make_pair(CString("EnumerableString"), globalConst->GetEnumerableString()));
    vec.push_back(std::make_pair(CString("ConfigurableString"), globalConst->GetConfigurableString()));
    vec.push_back(std::make_pair(CString("NameString"), globalConst->GetNameString()));
    vec.push_back(std::make_pair(CString("ValueOfString"), globalConst->GetValueOfString()));
    vec.push_back(std::make_pair(CString("ToStringString"), globalConst->GetToStringString()));
    vec.push_back(std::make_pair(CString("ToLocaleStringString"), globalConst->GetToLocaleStringString()));
    vec.push_back(std::make_pair(CString("UndefinedString"), globalConst->GetUndefinedString()));
    vec.push_back(std::make_pair(CString("NullString"), globalConst->GetNullString()));
    vec.push_back(std::make_pair(CString("TrueString"), globalConst->GetTrueString()));
    vec.push_back(std::make_pair(CString("FalseString"), globalConst->GetFalseString()));
    vec.push_back(std::make_pair(CString("RegisterSymbols"), GetRegisterSymbols().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ThrowTypeError"), GetThrowTypeError().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("GetPrototypeOfString"), globalConst->GetGetPrototypeOfString()));
    vec.push_back(std::make_pair(CString("SetPrototypeOfString"), globalConst->GetSetPrototypeOfString()));
    vec.push_back(std::make_pair(CString("IsExtensibleString"), globalConst->GetIsExtensibleString()));
    vec.push_back(std::make_pair(CString("PreventExtensionsString"), globalConst->GetPreventExtensionsString()));
    vec.push_back(
        std::make_pair(CString("GetOwnPropertyDescriptorString"), globalConst->GetGetOwnPropertyDescriptorString()));
    vec.push_back(std::make_pair(CString("DefinePropertyString"), globalConst->GetDefinePropertyString()));
    vec.push_back(std::make_pair(CString("HasString"), globalConst->GetHasString()));
    vec.push_back(std::make_pair(CString("DeletePropertyString"), globalConst->GetDeletePropertyString()));
    vec.push_back(std::make_pair(CString("EnumerateString"), globalConst->GetEnumerateString()));
    vec.push_back(std::make_pair(CString("OwnKeysString"), globalConst->GetOwnKeysString()));
    vec.push_back(std::make_pair(CString("ApplyString"), globalConst->GetApplyString()));
    vec.push_back(std::make_pair(CString("ProxyString"), globalConst->GetProxyString()));
    vec.push_back(std::make_pair(CString("RevokeString"), globalConst->GetRevokeString()));
    vec.push_back(std::make_pair(CString("ProxyConstructString"), globalConst->GetProxyConstructString()));
    vec.push_back(std::make_pair(CString("ProxyCallString"), globalConst->GetProxyCallString()));
    vec.push_back(std::make_pair(CString("DoneString"), globalConst->GetDoneString()));
    vec.push_back(std::make_pair(CString("NegativeZeroString"), globalConst->GetNegativeZeroString()));
    vec.push_back(std::make_pair(CString("NextString"), globalConst->GetNextString()));
    vec.push_back(std::make_pair(CString("PromiseThenString"), globalConst->GetPromiseThenString()));
    vec.push_back(std::make_pair(CString("PromiseFunction"), GetPromiseFunction().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("PromiseReactionJob"), GetPromiseReactionJob().GetTaggedValue()));
    vec.push_back(
        std::make_pair(CString("PromiseResolveThenableJob"), GetPromiseResolveThenableJob().GetTaggedValue()));
    vec.push_back(std::make_pair(CString("ScriptJobString"), globalConst->GetScriptJobString()));
    vec.push_back(std::make_pair(CString("PromiseString"), globalConst->GetPromiseString()));
    vec.push_back(std::make_pair(CString("IdentityString"), globalConst->GetIdentityString()));
    vec.push_back(std::make_pair(CString("AsyncFunctionString"), globalConst->GetAsyncFunctionString()));
    vec.push_back(std::make_pair(CString("ThrowerString"), globalConst->GetThrowerString()));
    vec.push_back(std::make_pair(CString("Undefined"), globalConst->GetUndefined()));
}

void JSDataView::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                 std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("data-view"), GetDataView()));
    vec.push_back(std::make_pair(CString("buffer"), GetViewedArrayBuffer()));
    vec.push_back(std::make_pair(CString("byte-length"), GetByteLength()));
    vec.push_back(std::make_pair(CString("byte-offset"), GetByteOffset()));
}

void JSArrayBuffer::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("byte-length"), GetArrayBufferByteLength()));
    vec.push_back(std::make_pair(CString("buffer-data"), GetArrayBufferData()));
    vec.push_back(std::make_pair(CString("shared"), GetShared()));
}

void PromiseReaction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                      std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("promise-capability"), GetPromiseCapability()));
    vec.push_back(std::make_pair(CString("type"), GetType()));
    vec.push_back(std::make_pair(CString("handler"), GetHandler()));
}

void PromiseCapability::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                        std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("promise"), GetPromise()));
    vec.push_back(std::make_pair(CString("resolve"), GetResolve()));
    vec.push_back(std::make_pair(CString("reject"), GetReject()));
}

void PromiseIteratorRecord::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                            std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("iterator"), GetIterator()));
    vec.push_back(std::make_pair(CString("done"), GetDone()));
}

void PromiseRecord::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("value"), GetValue()));
}

void ResolvingFunctionsRecord::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                               std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("resolve-function"), GetResolveFunction()));
    vec.push_back(std::make_pair(CString("reject-function"), GetRejectFunction()));
}

void JSPromise::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("promise-state"), GetPromiseState()));
    vec.push_back(std::make_pair(CString("promise-result"), GetPromiseResult()));
    vec.push_back(std::make_pair(CString("promise-fulfill-reactions"), GetPromiseFulfillReactions()));
    vec.push_back(std::make_pair(CString("promise-reject-reactions"), GetPromiseRejectReactions()));
    vec.push_back(std::make_pair(CString("promise-is-handled"), GetPromiseIsHandled()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSPromiseReactionsFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                                 std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("promise"), GetPromise()));
    vec.push_back(std::make_pair(CString("already-resolved"), GetAlreadyResolved()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSPromiseExecutorFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                                std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("capability"), GetCapability()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSPromiseAllResolveElementFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                                         std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("index"), GetIndex()));
    vec.push_back(std::make_pair(CString("values"), GetValues()));
    vec.push_back(std::make_pair(CString("capabilities"), GetCapabilities()));
    vec.push_back(std::make_pair(CString("remaining-elements"), GetRemainingElements()));
    vec.push_back(std::make_pair(CString("already-called"), GetAlreadyCalled()));
    JSObject::DumpForSnapshot(thread, vec);
}

void MicroJobQueue::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                    std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("promise-job-queue"), GetPromiseJobQueue()));
    vec.push_back(std::make_pair(CString("script-job-queue"), GetScriptJobQueue()));
}

void PendingJob::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                 std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("job"), GetJob()));
    vec.push_back(std::make_pair(CString("arguments"), GetArguments()));
}

void CompletionRecord::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                       std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("type"), GetType()));
    vec.push_back(std::make_pair(CString("value"), GetValue()));
}

void JSProxyRevocFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                           std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("RevocableProxy"), GetRevocableProxy()));
}

void JSAsyncFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                      std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    JSFunction::DumpForSnapshot(thread, vec);
}

void JSAsyncAwaitStatusFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                                 std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("AsyncContext"), GetAsyncContext()));
}

void JSGeneratorFunction::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                          std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    JSFunction::DumpForSnapshot(thread, vec);
}

void JSIntlBoundFunction::DumpForSnapshot(JSThread *thread,
                                          std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("NumberFormat"), GetNumberFormat()));
    vec.push_back(std::make_pair(CString("DateTimeFormat"), GetDateTimeFormat()));
    vec.push_back(std::make_pair(CString("Collator"), GetCollator()));
    JSObject::DumpForSnapshot(thread, vec);
}

void PropertyBox::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                  std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Value"), GetValue()));
}

void PrototypeHandler::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                       std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("HandlerInfo"), GetHandlerInfo()));
    vec.push_back(std::make_pair(CString("ProtoCell"), GetProtoCell()));
    vec.push_back(std::make_pair(CString("Holder"), GetHolder()));
}

void TransitionHandler::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                                        std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("HandlerInfo"), GetHandlerInfo()));
    vec.push_back(std::make_pair(CString("TransitionHClass"), GetTransitionHClass()));
}

void JSRealm::DumpForSnapshot([[maybe_unused]] JSThread *thread,
                              std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Value"), GetValue()));
    vec.push_back(std::make_pair(CString("GLobalEnv"), GetGlobalEnv()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSIntl::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("FallbackSymbol"), GetFallbackSymbol()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSLocale::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("IcuField"), GetIcuField()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSDateTimeFormat::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Locale"), GetLocale()));
    vec.push_back(std::make_pair(CString("Calendar"), GetCalendar()));
    vec.push_back(std::make_pair(CString("NumberingSystem"), GetNumberingSystem()));
    vec.push_back(std::make_pair(CString("TimeZone"), GetTimeZone()));
    vec.push_back(std::make_pair(CString("HourCycle"), GetHourCycle()));
    vec.push_back(std::make_pair(CString("LocaleIcu"), GetLocaleIcu()));
    vec.push_back(std::make_pair(CString("SimpleDateTimeFormatIcu"), GetSimpleDateTimeFormatIcu()));
    vec.push_back(std::make_pair(CString("Iso8601"), GetIso8601()));
    vec.push_back(std::make_pair(CString("DateStyle"), GetDateStyle()));
    vec.push_back(std::make_pair(CString("TimeStyle"), GetTimeStyle()));
    vec.push_back(std::make_pair(CString("BoundFormat"), GetBoundFormat()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSRelativeTimeFormat::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Locale"), GetLocale()));
    vec.push_back(std::make_pair(CString("InitializedRelativeTimeFormat"), GetInitializedRelativeTimeFormat()));
    vec.push_back(std::make_pair(CString("NumberingSystem"), GetNumberingSystem()));
    vec.push_back(std::make_pair(CString("Style"), GetStyle()));
    vec.push_back(std::make_pair(CString("Numeric"), GetNumeric()));
    vec.push_back(std::make_pair(CString("AvailableLocales"), GetAvailableLocales()));
    vec.push_back(std::make_pair(CString("IcuField"), GetIcuField()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSNumberFormat::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Locale"), GetLocale()));
    vec.push_back(std::make_pair(CString("NumberingSystem"), GetNumberingSystem()));
    vec.push_back(std::make_pair(CString("Style"), GetStyle()));
    vec.push_back(std::make_pair(CString("Currency"), GetCurrency()));
    vec.push_back(std::make_pair(CString("CurrencyDisplay"), GetCurrencyDisplay()));
    vec.push_back(std::make_pair(CString("CurrencySign"), GetCurrencySign()));
    vec.push_back(std::make_pair(CString("Unit"), GetUnit()));
    vec.push_back(std::make_pair(CString("UnitDisplay"), GetUnitDisplay()));
    vec.push_back(std::make_pair(CString("MinimumIntegerDigits"), GetMinimumIntegerDigits()));
    vec.push_back(std::make_pair(CString("MinimumFractionDigits"), GetMinimumFractionDigits()));
    vec.push_back(std::make_pair(CString("MaximumFractionDigits"), GetMaximumFractionDigits()));
    vec.push_back(std::make_pair(CString("MinimumSignificantDigits"), GetMinimumSignificantDigits()));
    vec.push_back(std::make_pair(CString("MaximumSignificantDigits"), GetMaximumSignificantDigits()));
    vec.push_back(std::make_pair(CString("UseGrouping"), GetUseGrouping()));
    vec.push_back(std::make_pair(CString("RoundingType"), GetRoundingType()));
    vec.push_back(std::make_pair(CString("Notation"), GetNotation()));
    vec.push_back(std::make_pair(CString("CompactDisplay"), GetCompactDisplay()));
    vec.push_back(std::make_pair(CString("SignDisplay"), GetSignDisplay()));
    vec.push_back(std::make_pair(CString("BoundFormat"), GetBoundFormat()));
    vec.push_back(std::make_pair(CString("IcuField"), GetIcuField()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSCollator::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("IcuField"), GetIcuField()));
    vec.push_back(std::make_pair(CString("Locale"), GetLocale()));
    vec.push_back(std::make_pair(CString("Usage"), GetUsage()));
    vec.push_back(std::make_pair(CString("Sensitivity"), GetSensitivity()));
    vec.push_back(std::make_pair(CString("IgnorePunctuation"), GetIgnorePunctuation()));
    vec.push_back(std::make_pair(CString("Collation"), GetCollation()));
    vec.push_back(std::make_pair(CString("Numeric"), GetNumeric()));
    vec.push_back(std::make_pair(CString("CaseFirst"), GetCaseFirst()));
    vec.push_back(std::make_pair(CString("BoundCompare"), GetBoundCompare()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSPluralRules::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Locale"), GetLocale()));
    vec.push_back(std::make_pair(CString("InitializedPluralRules"), GetInitializedPluralRules()));
    vec.push_back(std::make_pair(CString("Type"), GetType()));
    vec.push_back(std::make_pair(CString("MinimumIntegerDigits"), GetMinimumIntegerDigits()));
    vec.push_back(std::make_pair(CString("MinimumFractionDigits"), GetMinimumFractionDigits()));
    vec.push_back(std::make_pair(CString("MaximumFractionDigits"), GetMaximumFractionDigits()));
    vec.push_back(std::make_pair(CString("MinimumSignificantDigits"), GetMinimumSignificantDigits()));
    vec.push_back(std::make_pair(CString("MaximumSignificantDigits"), GetMaximumSignificantDigits()));
    vec.push_back(std::make_pair(CString("RoundingType"), GetRoundingType()));
    vec.push_back(std::make_pair(CString("IcuPR"), GetIcuPR()));
    vec.push_back(std::make_pair(CString("IcuNF"), GetIcuNF()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSGeneratorObject::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("GeneratorState"), GetGeneratorState()));
    vec.push_back(std::make_pair(CString("GeneratorContext"), GetGeneratorContext()));
    vec.push_back(std::make_pair(CString("ResumeResult"), GetResumeResult()));
    vec.push_back(std::make_pair(CString("ResumeMode"), GetResumeMode()));
    JSObject::DumpForSnapshot(thread, vec);
}

void JSAsyncFuncObject::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Promise"), GetPromise()));
}

void GeneratorContext::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("RegsArray"), GetRegsArray()));
    vec.push_back(std::make_pair(CString("Method"), GetMethod()));
    vec.push_back(std::make_pair(CString("Acc"), GetAcc()));
    vec.push_back(std::make_pair(CString("NRegs"), GetNRegs()));
    vec.push_back(std::make_pair(CString("BCOffset"), GetBCOffset()));
    vec.push_back(std::make_pair(CString("GeneratorObject"), GetGeneratorObject()));
    vec.push_back(std::make_pair(CString("LexicalEnv"), GetLexicalEnv()));
}

void ProtoChangeMarker::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Promise"), JSTaggedValue(GetHasChanged())));
}

void ProtoChangeDetails::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("ChangeListener"), GetChangeListener()));
    vec.push_back(std::make_pair(CString("RegisterIndex"), GetRegisterIndex()));
}

void LexicalFunction::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Name"), GetName()));
    vec.push_back(std::make_pair(CString("NumberVRegs"), GetNumberVRegs()));
    vec.push_back(std::make_pair(CString("NumberICSlots"), GetNumberICSlots()));
    vec.push_back(std::make_pair(CString("Bytecode"), GetBytecode()));
    vec.push_back(std::make_pair(CString("Program"), GetProgram()));
}

void JSFunctionExtraInfo::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("Callback"), GetCallback()));
    vec.push_back(std::make_pair(CString("Data"), GetData()));
}

void MachineCode::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("InstructionSizeInBytes"), GetInstructionSizeInBytes()));
}

void EcmaModule::DumpForSnapshot(JSThread *thread, std::vector<std::pair<CString, JSTaggedValue>> &vec) const
{
    vec.push_back(std::make_pair(CString("NameDictionary"), GetNameDictionary()));
}
}  // namespace panda::ecmascript
