/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QDECLARATIVEVIEW_H
#define QDECLARATIVEVIEW_H

#include <QtCore/qdatetime.h>
#include <QtCore/qurl.h>
#include <QtWidgets/qgraphicssceneevent.h>
#include <QtWidgets/qgraphicsview.h>
#include <QtWidgets/qwidget.h>
#include <QtDeclarative/qdeclarativedebug.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QGraphicsObject;
class QDeclarativeEngine;
class QDeclarativeContext;
class QDeclarativeError;
class QDeclarativeItem;


class QDeclarativeViewPrivate;
class Q_QTQUICK1_EXPORT QDeclarativeView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource DESIGNABLE true)
    Q_ENUMS(ResizeMode Status)
public:
    explicit QDeclarativeView(QWidget *parent = 0);
    QDeclarativeView(const QUrl &source, QWidget *parent = 0);
    virtual ~QDeclarativeView();

    QUrl source() const;

    QDeclarativeEngine* engine() const;
    QDeclarativeContext* rootContext() const;

    QGraphicsObject *rootObject() const;

    enum ResizeMode { SizeViewToRootObject, SizeRootObjectToView };
    ResizeMode resizeMode() const;
    void setResizeMode(ResizeMode);

    enum Status { Null, Ready, Loading, Error };
    Status status() const;

    QList<QDeclarativeError> errors() const;

    QSize sizeHint() const;
    QSize initialSize() const;

public Q_SLOTS:
    void setSource(const QUrl&);

Q_SIGNALS:
    void sceneResized(QSize size); // ???
    void statusChanged(QDeclarativeView::Status);
    void initialSizeChanged(QSize size);

private Q_SLOTS:
    void continueExecute();

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *event);
    virtual void timerEvent(QTimerEvent*);
    virtual void setRootObject(QObject *obj);
    virtual bool eventFilter(QObject *watched, QEvent *e);

private:
    Q_DISABLE_COPY(QDeclarativeView)
    Q_DECLARE_PRIVATE(QDeclarativeView)
// Accessibility support:
    friend class QAccessibleDeclarativeView;
    QDeclarativeItem *accessibleRootItem() const;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QDECLARATIVEVIEW_H
