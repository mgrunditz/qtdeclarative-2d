/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtQuick module of the Qt Toolkit.
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

#ifndef QQUICKSTYLEDTEXT_H
#define QQUICKSTYLEDTEXT_H
#include <QPixmap>
#include <QSize>
#include <QPointF>
#include <QList>
#include <QUrl>
//#include <QtQuick/private/qquickpixmapcache_p.h>

QT_BEGIN_NAMESPACE

class QQuickStyledTextImgTag;
class QQuickStyledTextPrivate;
class QString;
class QQmlContext;

class Q_AUTOTEST_EXPORT QQuickStyledTextImgTag
{
public:
    QQuickStyledTextImgTag()
        : position(0), align(QQuickStyledTextImgTag::Bottom), pix(0)
    { }

    ~QQuickStyledTextImgTag() { delete pix; }

    enum Align {
        Bottom,
        Middle,
        Top
    };

    QUrl url;
    QPointF pos;
    QSize size;
    int position;
    Align align;
    QPixmap *pix;
};

class Q_AUTOTEST_EXPORT QQuickStyledText
{
public:
    static void parse(const QString &string, QTextLayout &layout,
                      QList<QQuickStyledTextImgTag*> &imgTags,
                      const QUrl &baseUrl,
                      QQmlContext *context,
                      bool preloadImages,
                      bool *fontSizeModified);

private:
    QQuickStyledText(const QString &string, QTextLayout &layout,
                           QList<QQuickStyledTextImgTag*> &imgTags,
                           const QUrl &baseUrl,
                           QQmlContext *context,
                           bool preloadImages,
                           bool *fontSizeModified);
    ~QQuickStyledText();

    QQuickStyledTextPrivate *d;
};

QT_END_NAMESPACE

#endif
