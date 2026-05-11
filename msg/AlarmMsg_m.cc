//
// Generated file, do not edit! Created by opp_msgtool 6.2 from msg/AlarmMsg.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include <memory>
#include <type_traits>
#include "AlarmMsg_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i = 0; i < n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace forestfiresim {

Register_Class(AlarmMsg)

AlarmMsg::AlarmMsg(const char *name, short kind) : ::omnetpp::cMessage(name, kind)
{
}

AlarmMsg::AlarmMsg(const AlarmMsg& other) : ::omnetpp::cMessage(other)
{
    copy(other);
}

AlarmMsg::~AlarmMsg()
{
}

AlarmMsg& AlarmMsg::operator=(const AlarmMsg& other)
{
    if (this == &other) return *this;
    ::omnetpp::cMessage::operator=(other);
    copy(other);
    return *this;
}

void AlarmMsg::copy(const AlarmMsg& other)
{
    this->sensorId = other.sensorId;
    this->timestamp = other.timestamp;
    this->sensorReading = other.sensorReading;
    this->isFire_ = other.isFire_;
    this->isFalseAlarm_ = other.isFalseAlarm_;
    this->zoneId = other.zoneId;
}

void AlarmMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::omnetpp::cMessage::parsimPack(b);
    doParsimPacking(b,this->sensorId);
    doParsimPacking(b,this->timestamp);
    doParsimPacking(b,this->sensorReading);
    doParsimPacking(b,this->isFire_);
    doParsimPacking(b,this->isFalseAlarm_);
    doParsimPacking(b,this->zoneId);
}

void AlarmMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::omnetpp::cMessage::parsimUnpack(b);
    doParsimUnpacking(b,this->sensorId);
    doParsimUnpacking(b,this->timestamp);
    doParsimUnpacking(b,this->sensorReading);
    doParsimUnpacking(b,this->isFire_);
    doParsimUnpacking(b,this->isFalseAlarm_);
    doParsimUnpacking(b,this->zoneId);
}

int AlarmMsg::getSensorId() const
{
    return this->sensorId;
}

void AlarmMsg::setSensorId(int sensorId)
{
    this->sensorId = sensorId;
}

double AlarmMsg::getTimestamp() const
{
    return this->timestamp;
}

void AlarmMsg::setTimestamp(double timestamp)
{
    this->timestamp = timestamp;
}

double AlarmMsg::getSensorReading() const
{
    return this->sensorReading;
}

void AlarmMsg::setSensorReading(double sensorReading)
{
    this->sensorReading = sensorReading;
}

bool AlarmMsg::isFire() const
{
    return this->isFire_;
}

void AlarmMsg::setIsFire(bool isFire)
{
    this->isFire_ = isFire;
}

bool AlarmMsg::isFalseAlarm() const
{
    return this->isFalseAlarm_;
}

void AlarmMsg::setIsFalseAlarm(bool isFalseAlarm)
{
    this->isFalseAlarm_ = isFalseAlarm;
}

int AlarmMsg::getZoneId() const
{
    return this->zoneId;
}

void AlarmMsg::setZoneId(int zoneId)
{
    this->zoneId = zoneId;
}

class AlarmMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertyNames;
    enum FieldConstants {
        FIELD_sensorId,
        FIELD_timestamp,
        FIELD_sensorReading,
        FIELD_isFire,
        FIELD_isFalseAlarm,
        FIELD_zoneId,
    };
  public:
    AlarmMsgDescriptor();
    virtual ~AlarmMsgDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyName) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyName) const override;
    virtual int getFieldArraySize(omnetpp::any_ptr object, int field) const override;
    virtual void setFieldArraySize(omnetpp::any_ptr object, int field, int size) const override;

    virtual const char *getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const override;
    virtual std::string getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const override;
    virtual omnetpp::cValue getFieldValue(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual omnetpp::any_ptr getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const override;
    virtual void setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const override;
};

Register_ClassDescriptor(AlarmMsgDescriptor)

AlarmMsgDescriptor::AlarmMsgDescriptor() : omnetpp::cClassDescriptor(omnetpp::opp_typename(typeid(forestfiresim::AlarmMsg)), "omnetpp::cMessage")
{
    propertyNames = nullptr;
}

AlarmMsgDescriptor::~AlarmMsgDescriptor()
{
    delete[] propertyNames;
}

bool AlarmMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<AlarmMsg *>(obj)!=nullptr;
}

const char **AlarmMsgDescriptor::getPropertyNames() const
{
    if (!propertyNames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
        const char **baseNames = base ? base->getPropertyNames() : nullptr;
        propertyNames = mergeLists(baseNames, names);
    }
    return propertyNames;
}

const char *AlarmMsgDescriptor::getProperty(const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? base->getProperty(propertyName) : nullptr;
}

int AlarmMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    return base ? 6+base->getFieldCount() : 6;
}

unsigned int AlarmMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeFlags(field);
        field -= base->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,    // FIELD_sensorId
        FD_ISEDITABLE,    // FIELD_timestamp
        FD_ISEDITABLE,    // FIELD_sensorReading
        FD_ISEDITABLE,    // FIELD_isFire
        FD_ISEDITABLE,    // FIELD_isFalseAlarm
        FD_ISEDITABLE,    // FIELD_zoneId
    };
    return (field >= 0 && field < 6) ? fieldTypeFlags[field] : 0;
}

const char *AlarmMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldName(field);
        field -= base->getFieldCount();
    }
    static const char *fieldNames[] = {
        "sensorId",
        "timestamp",
        "sensorReading",
        "isFire",
        "isFalseAlarm",
        "zoneId",
    };
    return (field >= 0 && field < 6) ? fieldNames[field] : nullptr;
}

int AlarmMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    int baseIndex = base ? base->getFieldCount() : 0;
    if (strcmp(fieldName, "sensorId") == 0) return baseIndex + 0;
    if (strcmp(fieldName, "timestamp") == 0) return baseIndex + 1;
    if (strcmp(fieldName, "sensorReading") == 0) return baseIndex + 2;
    if (strcmp(fieldName, "isFire") == 0) return baseIndex + 3;
    if (strcmp(fieldName, "isFalseAlarm") == 0) return baseIndex + 4;
    if (strcmp(fieldName, "zoneId") == 0) return baseIndex + 5;
    return base ? base->findField(fieldName) : -1;
}

const char *AlarmMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldTypeString(field);
        field -= base->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "int",    // FIELD_sensorId
        "double",    // FIELD_timestamp
        "double",    // FIELD_sensorReading
        "bool",    // FIELD_isFire
        "bool",    // FIELD_isFalseAlarm
        "int",    // FIELD_zoneId
    };
    return (field >= 0 && field < 6) ? fieldTypeStrings[field] : nullptr;
}

const char **AlarmMsgDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldPropertyNames(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *AlarmMsgDescriptor::getFieldProperty(int field, const char *propertyName) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldProperty(field, propertyName);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int AlarmMsgDescriptor::getFieldArraySize(omnetpp::any_ptr object, int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldArraySize(object, field);
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        default: return 0;
    }
}

void AlarmMsgDescriptor::setFieldArraySize(omnetpp::any_ptr object, int field, int size) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldArraySize(object, field, size);
            return;
        }
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set array size of field %d of class 'AlarmMsg'", field);
    }
}

const char *AlarmMsgDescriptor::getFieldDynamicTypeString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldDynamicTypeString(object,field,i);
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string AlarmMsgDescriptor::getFieldValueAsString(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValueAsString(object,field,i);
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        case FIELD_sensorId: return long2string(pp->getSensorId());
        case FIELD_timestamp: return double2string(pp->getTimestamp());
        case FIELD_sensorReading: return double2string(pp->getSensorReading());
        case FIELD_isFire: return bool2string(pp->isFire());
        case FIELD_isFalseAlarm: return bool2string(pp->isFalseAlarm());
        case FIELD_zoneId: return long2string(pp->getZoneId());
        default: return "";
    }
}

void AlarmMsgDescriptor::setFieldValueAsString(omnetpp::any_ptr object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValueAsString(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        case FIELD_sensorId: pp->setSensorId(string2long(value)); break;
        case FIELD_timestamp: pp->setTimestamp(string2double(value)); break;
        case FIELD_sensorReading: pp->setSensorReading(string2double(value)); break;
        case FIELD_isFire: pp->setIsFire(string2bool(value)); break;
        case FIELD_isFalseAlarm: pp->setIsFalseAlarm(string2bool(value)); break;
        case FIELD_zoneId: pp->setZoneId(string2long(value)); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AlarmMsg'", field);
    }
}

omnetpp::cValue AlarmMsgDescriptor::getFieldValue(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldValue(object,field,i);
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        case FIELD_sensorId: return pp->getSensorId();
        case FIELD_timestamp: return pp->getTimestamp();
        case FIELD_sensorReading: return pp->getSensorReading();
        case FIELD_isFire: return pp->isFire();
        case FIELD_isFalseAlarm: return pp->isFalseAlarm();
        case FIELD_zoneId: return pp->getZoneId();
        default: throw omnetpp::cRuntimeError("Cannot return field %d of class 'AlarmMsg' as cValue -- field index out of range?", field);
    }
}

void AlarmMsgDescriptor::setFieldValue(omnetpp::any_ptr object, int field, int i, const omnetpp::cValue& value) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldValue(object, field, i, value);
            return;
        }
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        case FIELD_sensorId: pp->setSensorId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        case FIELD_timestamp: pp->setTimestamp(value.doubleValue()); break;
        case FIELD_sensorReading: pp->setSensorReading(value.doubleValue()); break;
        case FIELD_isFire: pp->setIsFire(value.boolValue()); break;
        case FIELD_isFalseAlarm: pp->setIsFalseAlarm(value.boolValue()); break;
        case FIELD_zoneId: pp->setZoneId(omnetpp::checked_int_cast<int>(value.intValue())); break;
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AlarmMsg'", field);
    }
}

const char *AlarmMsgDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructName(field);
        field -= base->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    };
}

omnetpp::any_ptr AlarmMsgDescriptor::getFieldStructValuePointer(omnetpp::any_ptr object, int field, int i) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount())
            return base->getFieldStructValuePointer(object, field, i);
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        default: return omnetpp::any_ptr(nullptr);
    }
}

void AlarmMsgDescriptor::setFieldStructValuePointer(omnetpp::any_ptr object, int field, int i, omnetpp::any_ptr ptr) const
{
    omnetpp::cClassDescriptor *base = getBaseClassDescriptor();
    if (base) {
        if (field < base->getFieldCount()){
            base->setFieldStructValuePointer(object, field, i, ptr);
            return;
        }
        field -= base->getFieldCount();
    }
    AlarmMsg *pp = omnetpp::fromAnyPtr<AlarmMsg>(object); (void)pp;
    switch (field) {
        default: throw omnetpp::cRuntimeError("Cannot set field %d of class 'AlarmMsg'", field);
    }
}

}  // namespace forestfiresim

namespace omnetpp {

}  // namespace omnetpp

