/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QQMLINCUBATOR_H
#define QQMLINCUBATOR_H

#include <QtQml/qtqmlglobal.h>
#include <QtQml/qqmlerror.h>

QT_BEGIN_NAMESPACE


class QQmlEngine;

class QQmlIncubatorPrivate;
class Q_QML_EXPORT QQmlIncubator
{
    Q_DISABLE_COPY(QQmlIncubator)
public:
    enum IncubationMode {
        Asynchronous,
        AsynchronousIfNested,
        Synchronous
    };
    enum Status {
        Null,
        Ready,
        Loading,
        Error
    };

    QQmlIncubator(IncubationMode = Asynchronous);
    virtual ~QQmlIncubator();

    void clear();
    void forceCompletion();

    bool isNull() const;
    bool isReady() const;
    bool isError() const;
    bool isLoading() const;

    QList<QQmlError> errors() const;

    IncubationMode incubationMode() const;

    Status status() const;

    QObject *object() const;

protected:
    virtual void statusChanged(Status);
    virtual void setInitialState(QObject *);

private:
    friend class QQmlComponent;
    friend class QQmlEnginePrivate;
    friend class QQmlIncubatorPrivate;
    QQmlIncubatorPrivate *d;
};

class QQmlEnginePrivate;
class Q_QML_EXPORT QQmlIncubationController
{
    Q_DISABLE_COPY(QQmlIncubationController)
public:
    QQmlIncubationController();
    virtual ~QQmlIncubationController();

    QQmlEngine *engine() const;
    int incubatingObjectCount() const;

    void incubateFor(int msecs);
    void incubateWhile(volatile bool *flag, int msecs=0);

protected:
    virtual void incubatingObjectCountChanged(int);

private:
    friend class QQmlEngine;
    friend class QQmlEnginePrivate;
    friend class QQmlIncubatorPrivate;
    QQmlEnginePrivate *d;
};

QT_END_NAMESPACE

#endif // QQMLINCUBATOR_H
