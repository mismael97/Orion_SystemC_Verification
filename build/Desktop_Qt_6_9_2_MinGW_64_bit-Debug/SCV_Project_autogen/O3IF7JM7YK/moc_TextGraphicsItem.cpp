/****************************************************************************
** Meta object code from reading C++ file 'TextGraphicsItem.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../include/graphics/TextGraphicsItem.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TextGraphicsItem.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN16TextGraphicsItemE_t {};
} // unnamed namespace

template <> constexpr inline auto TextGraphicsItem::qt_create_metaobjectdata<qt_meta_tag_ZN16TextGraphicsItemE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "TextGraphicsItem",
        "textChanged",
        "",
        "newText",
        "textEditingFinished",
        "positionChanged",
        "newPosition"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'textChanged'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'textEditingFinished'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'positionChanged'
        QtMocHelpers::SignalData<void(const QPointF &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QPointF, 6 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<TextGraphicsItem, qt_meta_tag_ZN16TextGraphicsItemE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject TextGraphicsItem::staticMetaObject = { {
    QMetaObject::SuperData::link<QGraphicsTextItem::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16TextGraphicsItemE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16TextGraphicsItemE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16TextGraphicsItemE_t>.metaTypes,
    nullptr
} };

void TextGraphicsItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<TextGraphicsItem *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->textChanged((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->textEditingFinished(); break;
        case 2: _t->positionChanged((*reinterpret_cast< std::add_pointer_t<QPointF>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (TextGraphicsItem::*)(const QString & )>(_a, &TextGraphicsItem::textChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (TextGraphicsItem::*)()>(_a, &TextGraphicsItem::textEditingFinished, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (TextGraphicsItem::*)(const QPointF & )>(_a, &TextGraphicsItem::positionChanged, 2))
            return;
    }
}

const QMetaObject *TextGraphicsItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TextGraphicsItem::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16TextGraphicsItemE_t>.strings))
        return static_cast<void*>(this);
    return QGraphicsTextItem::qt_metacast(_clname);
}

int TextGraphicsItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsTextItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void TextGraphicsItem::textChanged(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void TextGraphicsItem::textEditingFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void TextGraphicsItem::positionChanged(const QPointF & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
