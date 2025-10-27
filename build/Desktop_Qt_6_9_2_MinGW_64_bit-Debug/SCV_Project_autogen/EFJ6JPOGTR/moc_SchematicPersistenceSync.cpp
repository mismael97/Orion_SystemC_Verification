/****************************************************************************
** Meta object code from reading C++ file 'SchematicPersistenceSync.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/persistence/SchematicPersistenceSync.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SchematicPersistenceSync.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN24SchematicPersistenceSyncE_t {};
} // unnamed namespace

template <> constexpr inline auto SchematicPersistenceSync::qt_create_metaobjectdata<qt_meta_tag_ZN24SchematicPersistenceSyncE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SchematicPersistenceSync",
        "synchronizationStarted",
        "",
        "synchronizationCompleted",
        "synchronizationError",
        "error",
        "componentSynced",
        "componentId",
        "connectionSynced",
        "connectionId",
        "textItemSynced",
        "textItemId",
        "onBatchSyncTimer",
        "onComponentPositionChanged",
        "newPosition",
        "onComponentSizeChanged",
        "newSize",
        "onComponentColorChanged",
        "newColor",
        "onComponentRotationChanged",
        "newRotation",
        "onComponentOpacityChanged",
        "newOpacity",
        "onComponentVisibilityChanged",
        "newVisibility",
        "onTextItemChanged",
        "newText",
        "onTextItemPositionChanged"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'synchronizationStarted'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'synchronizationCompleted'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'synchronizationError'
        QtMocHelpers::SignalData<void(const QString &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 5 },
        }}),
        // Signal 'componentSynced'
        QtMocHelpers::SignalData<void(const QString &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 7 },
        }}),
        // Signal 'connectionSynced'
        QtMocHelpers::SignalData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Signal 'textItemSynced'
        QtMocHelpers::SignalData<void(const QString &)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 11 },
        }}),
        // Slot 'onBatchSyncTimer'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onComponentPositionChanged'
        QtMocHelpers::SlotData<void(const QPointF &)>(13, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPointF, 14 },
        }}),
        // Slot 'onComponentSizeChanged'
        QtMocHelpers::SlotData<void(const QSizeF &)>(15, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QSizeF, 16 },
        }}),
        // Slot 'onComponentColorChanged'
        QtMocHelpers::SlotData<void(const QColor &)>(17, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QColor, 18 },
        }}),
        // Slot 'onComponentRotationChanged'
        QtMocHelpers::SlotData<void(qreal)>(19, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QReal, 20 },
        }}),
        // Slot 'onComponentOpacityChanged'
        QtMocHelpers::SlotData<void(qreal)>(21, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QReal, 22 },
        }}),
        // Slot 'onComponentVisibilityChanged'
        QtMocHelpers::SlotData<void(bool)>(23, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Bool, 24 },
        }}),
        // Slot 'onTextItemChanged'
        QtMocHelpers::SlotData<void(const QString &)>(25, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QString, 26 },
        }}),
        // Slot 'onTextItemPositionChanged'
        QtMocHelpers::SlotData<void(const QPointF &)>(27, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::QPointF, 14 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SchematicPersistenceSync, qt_meta_tag_ZN24SchematicPersistenceSyncE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SchematicPersistenceSync::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN24SchematicPersistenceSyncE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN24SchematicPersistenceSyncE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN24SchematicPersistenceSyncE_t>.metaTypes,
    nullptr
} };

void SchematicPersistenceSync::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SchematicPersistenceSync *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->synchronizationStarted(); break;
        case 1: _t->synchronizationCompleted(); break;
        case 2: _t->synchronizationError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->componentSynced((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->connectionSynced((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 5: _t->textItemSynced((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 6: _t->onBatchSyncTimer(); break;
        case 7: _t->onComponentPositionChanged((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 8: _t->onComponentSizeChanged((*reinterpret_cast< std::add_pointer_t<QSizeF>>(_a[1]))); break;
        case 9: _t->onComponentColorChanged((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1]))); break;
        case 10: _t->onComponentRotationChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 11: _t->onComponentOpacityChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 12: _t->onComponentVisibilityChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 13: _t->onTextItemChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 14: _t->onTextItemPositionChanged((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SchematicPersistenceSync::*)()>(_a, &SchematicPersistenceSync::synchronizationStarted, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SchematicPersistenceSync::*)()>(_a, &SchematicPersistenceSync::synchronizationCompleted, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SchematicPersistenceSync::*)(const QString & )>(_a, &SchematicPersistenceSync::synchronizationError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SchematicPersistenceSync::*)(const QString & )>(_a, &SchematicPersistenceSync::componentSynced, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SchematicPersistenceSync::*)(const QString & )>(_a, &SchematicPersistenceSync::connectionSynced, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SchematicPersistenceSync::*)(const QString & )>(_a, &SchematicPersistenceSync::textItemSynced, 5))
            return;
    }
}

const QMetaObject *SchematicPersistenceSync::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SchematicPersistenceSync::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN24SchematicPersistenceSyncE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SchematicPersistenceSync::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 15)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 15;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 15)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 15;
    }
    return _id;
}

// SIGNAL 0
void SchematicPersistenceSync::synchronizationStarted()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SchematicPersistenceSync::synchronizationCompleted()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SchematicPersistenceSync::synchronizationError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void SchematicPersistenceSync::componentSynced(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void SchematicPersistenceSync::connectionSynced(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void SchematicPersistenceSync::textItemSynced(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}
QT_WARNING_POP
