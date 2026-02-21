/****************************************************************************
** Meta object code from reading C++ file 'simulatorpane.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.10.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../ui/simulatorpane.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'simulatorpane.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.10.2. It"
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
struct qt_meta_tag_ZN13SimulatorPaneE_t {};
} // unnamed namespace

template <> constexpr inline auto SimulatorPane::qt_create_metaobjectdata<qt_meta_tag_ZN13SimulatorPaneE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SimulatorPane",
        "onAddLed",
        "",
        "onAddButton",
        "onAddResistor",
        "onRemoveSelected",
        "onContextMenuRequested",
        "QPoint",
        "viewPos",
        "updateConnectionLines",
        "onZoomIn",
        "onZoomOut",
        "showZoomMenu",
        "showAddComponentMenu"
    };

    QtMocHelpers::UintData qt_methods {
        // Slot 'onAddLed'
        QtMocHelpers::SlotData<void()>(1, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddButton'
        QtMocHelpers::SlotData<void()>(3, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onAddResistor'
        QtMocHelpers::SlotData<void()>(4, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onRemoveSelected'
        QtMocHelpers::SlotData<void()>(5, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onContextMenuRequested'
        QtMocHelpers::SlotData<void(const QPoint &)>(6, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { 0x80000000 | 7, 8 },
        }}),
        // Slot 'updateConnectionLines'
        QtMocHelpers::SlotData<void()>(9, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomIn'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'onZoomOut'
        QtMocHelpers::SlotData<void()>(11, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showZoomMenu'
        QtMocHelpers::SlotData<void()>(12, 2, QMC::AccessPrivate, QMetaType::Void),
        // Slot 'showAddComponentMenu'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SimulatorPane, qt_meta_tag_ZN13SimulatorPaneE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SimulatorPane::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SimulatorPaneE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SimulatorPaneE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13SimulatorPaneE_t>.metaTypes,
    nullptr
} };

void SimulatorPane::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SimulatorPane *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->onAddLed(); break;
        case 1: _t->onAddButton(); break;
        case 2: _t->onAddResistor(); break;
        case 3: _t->onRemoveSelected(); break;
        case 4: _t->onContextMenuRequested((*reinterpret_cast<std::add_pointer_t<QPoint>>(_a[1]))); break;
        case 5: _t->updateConnectionLines(); break;
        case 6: _t->onZoomIn(); break;
        case 7: _t->onZoomOut(); break;
        case 8: _t->showZoomMenu(); break;
        case 9: _t->showAddComponentMenu(); break;
        default: ;
        }
    }
}

const QMetaObject *SimulatorPane::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SimulatorPane::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13SimulatorPaneE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SimulatorPane::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
