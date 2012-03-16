/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQMLABSTRACTBINDING_P_H
#define QQMLABSTRACTBINDING_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qsharedpointer.h>
#include <private/qtqmlglobal_p.h>
#include <private/qqmlproperty_p.h>
#include <private/qpointervaluepair_p.h>

QT_BEGIN_NAMESPACE

class Q_QML_PRIVATE_EXPORT QQmlAbstractBinding
{
public:
    typedef QWeakPointer<QQmlAbstractBinding> Pointer;

    QQmlAbstractBinding();

    virtual void destroy();

    virtual QString expression() const;

    enum Type { PropertyBinding, ValueTypeProxy };
    virtual Type bindingType() const { return PropertyBinding; }

    // Should return the encoded property index for the binding.  Should return this value
    // even if the binding is not enabled or added to an object.
    // Encoding is:  coreIndex | (valueTypeIndex << 24)
    virtual int propertyIndex() const = 0;
    // Should return the object for the binding.  Should return this object even if the
    // binding is not enabled or added to the object.
    virtual QObject *object() const = 0;

    void setEnabled(bool e) { setEnabled(e, QQmlPropertyPrivate::DontRemoveBinding); }
    virtual void setEnabled(bool, QQmlPropertyPrivate::WriteFlags) = 0;

    void update() { update(QQmlPropertyPrivate::DontRemoveBinding); }
    virtual void update(QQmlPropertyPrivate::WriteFlags) = 0;

    void addToObject();
    void removeFromObject();

    static inline Pointer getPointer(QQmlAbstractBinding *p);
    static void printBindingLoopError(QQmlProperty &prop);

protected:
    virtual ~QQmlAbstractBinding();
    void clear();

    // Called by QQmlPropertyPrivate to "move" a binding to a different property.
    // This is only used for alias properties, and only used by QQmlBinding not
    // V8 or V4 bindings.  The default implementation qFatal()'s to ensure that the
    // method is never called for V4 or V8 bindings.
    virtual void retargetBinding(QObject *, int);
private:
    Pointer weakPointer();

    friend class QQmlData;
    friend class QQmlComponentPrivate;
    friend class QQmlValueTypeProxyBinding;
    friend class QQmlPropertyPrivate;
    friend class QQmlVME;
    friend class QtSharedPointer::ExternalRefCount<QQmlAbstractBinding>;

    typedef QSharedPointer<QQmlAbstractBinding> SharedPointer;
    // To save memory, we also store the rarely used weakPointer() instance in here
    QPointerValuePair<QQmlAbstractBinding*, SharedPointer> m_mePtr;

    QQmlAbstractBinding **m_prevBinding;
    QQmlAbstractBinding  *m_nextBinding;
};

QQmlAbstractBinding::Pointer
QQmlAbstractBinding::getPointer(QQmlAbstractBinding *p)
{
    return p ? p->weakPointer() : Pointer();
}

QT_END_NAMESPACE

#endif // QQMLABSTRACTBINDING_P_H