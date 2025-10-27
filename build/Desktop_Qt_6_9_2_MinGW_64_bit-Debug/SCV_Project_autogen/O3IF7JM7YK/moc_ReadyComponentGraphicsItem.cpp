/****************************************************************************
** Meta object code from reading C++ file 'ReadyComponentGraphicsItem.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/graphics/ReadyComponentGraphicsItem.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ReadyComponentGraphicsItem.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN26ReadyComponentGraphicsItemE_t {};
} // unnamed namespace

template <> constexpr inline auto ReadyComponentGraphicsItem::qt_create_metaobjectdata<qt_meta_tag_ZN26ReadyComponentGraphicsItemE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "ReadyComponentGraphicsItem",
        "positionChanged",
        "",
        "newPosition",
        "sizeChanged",
        "newSize",
        "colorChanged",
        "newColor",
        "rotationChanged",
        "newRotation",
        "opacityChanged",
        "newOpacity",
        "visibilityChanged",
        "newVisibility",
        "componentDeleted"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'positionChanged'
        QtMocHelpers::SignalData<void(const QPointF &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QPointF, 3 },
        }}),
        // Signal 'sizeChanged'
        QtMocHelpers::SignalData<void(const QSizeF &)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QSizeF, 5 },
        }}),
        // Signal 'colorChanged'
        QtMocHelpers::SignalData<void(const QColor &)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QColor, 7 },
        }}),
        // Signal 'rotationChanged'
        QtMocHelpers::SignalData<void(qreal)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 9 },
        }}),
        // Signal 'opacityChanged'
        QtMocHelpers::SignalData<void(qreal)>(10, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QReal, 11 },
        }}),
        // Signal 'visibilityChanged'
        QtMocHelpers::SignalData<void(bool)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 13 },
        }}),
        // Signal 'componentDeleted'
        QtMocHelpers::SignalData<void()>(14, 2, QMC::AccessPublic, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<ReadyComponentGraphicsItem, qt_meta_tag_ZN26ReadyComponentGraphicsItemE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject ReadyComponentGraphicsItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN26ReadyComponentGraphicsItemE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN26ReadyComponentGraphicsItemE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN26ReadyComponentGraphicsItemE_t>.metaTypes,
    nullptr
} };

void ReadyComponentGraphicsItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<ReadyComponentGraphicsItem *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->positionChanged((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1]))); break;
        case 1: _t->sizeChanged((*reinterpret_cast< std::add_pointer_t<QSizeF>>(_a[1]))); break;
        case 2: _t->colorChanged((*reinterpret_cast< std::add_pointer_t<QColor>>(_a[1]))); break;
        case 3: _t->rotationChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 4: _t->opacityChanged((*reinterpret_cast< std::add_pointer_t<qreal>>(_a[1]))); break;
        case 5: _t->visibilityChanged((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 6: _t->componentDeleted(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (ReadyComponentGraphicsItem::*)(const QPointF & )>(_a, &ReadyComponentGraphicsItem::positionChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadyComponentGraphicsItem::*)(const QSizeF & )>(_a, &ReadyComponentGraphicsItem::sizeChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadyComponentGraphicsItem::*)(const QColor & )>(_a, &ReadyComponentGraphicsItem::colorChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadyComponentGraphicsItem::*)(qreal )>(_a, &ReadyComponentGraphicsItem::rotationChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadyComponentGraphicsItem::*)(qreal )>(_a, &ReadyComponentGraphicsItem::opacityChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadyComponentGraphicsItem::*)(bool )>(_a, &ReadyComponentGraphicsItem::visibilityChanged, 5))
            return;
        if (QtMocHelpers::indexOfMethod<void (ReadyComponentGraphicsItem::*)()>(_a, &ReadyComponentGraphicsItem::componentDeleted, 6))
            return;
    }
}

const QMetaObject *ReadyComponentGraphicsItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ReadyComponentGraphicsItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN26ReadyComponentGraphicsItemE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QGraphicsItem"))
        return static_cast< QGraphicsItem*>(this);
    if (!strcmp(_clname, "org.qt-project.Qt.QGraphicsItem"))
        return static_cast< QGraphicsItem*>(this);
    return QObject::qt_metacast(_clname);
}

int ReadyComponentGraphicsItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void ReadyComponentGraphicsItem::positionChanged(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void ReadyComponentGraphicsItem::sizeChanged(const QSizeF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void ReadyComponentGraphicsItem::colorChanged(const QColor & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void ReadyComponentGraphicsItem::rotationChanged(qreal _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}

// SIGNAL 4
void ReadyComponentGraphicsItem::opacityChanged(qreal _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 4, nullptr, _t1);
}

// SIGNAL 5
void ReadyComponentGraphicsItem::visibilityChanged(bool _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 5, nullptr, _t1);
}

// SIGNAL 6
void ReadyComponentGraphicsItem::componentDeleted()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}
QT_WARNING_POP
