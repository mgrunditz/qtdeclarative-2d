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

#include "qquickimagebase_p.h"
#include "qquickimagebase_p_p.h"

#include <QtGui/qguiapplication.h>

#include <QtQml/qqmlinfo.h>
#include <QtQml/qqmlfile.h>

QT_BEGIN_NAMESPACE

QQuickImageBase::QQuickImageBase(QQuickItem *parent)
: QQuickImplicitSizeItem(*(new QQuickImageBasePrivate), parent)
{
    setFlag(ItemHasContents);
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), SLOT(handleWindowChanged(QQuickWindow*)));
}

QQuickImageBase::QQuickImageBase(QQuickImageBasePrivate &dd, QQuickItem *parent)
: QQuickImplicitSizeItem(dd, parent)
{
    setFlag(ItemHasContents);
    connect(this, SIGNAL(windowChanged(QQuickWindow*)), SLOT(handleWindowChanged(QQuickWindow*)));
}

QQuickImageBase::~QQuickImageBase()
{
}

QQuickImageBase::Status QQuickImageBase::status() const
{
    Q_D(const QQuickImageBase);
    return d->status;
}


qreal QQuickImageBase::progress() const
{
    Q_D(const QQuickImageBase);
    return d->progress;
}


bool QQuickImageBase::asynchronous() const
{
    Q_D(const QQuickImageBase);
    return d->async;
}

void QQuickImageBase::setAsynchronous(bool async)
{
    Q_D(QQuickImageBase);
    if (d->async != async) {
        d->async = async;
        emit asynchronousChanged();
    }
}

QUrl QQuickImageBase::source() const
{
    Q_D(const QQuickImageBase);
    return d->url;
}

void QQuickImageBase::setSource(const QUrl &url)
{
    Q_D(QQuickImageBase);

    if (url == d->url)
        return;

    d->url = url;
    emit sourceChanged(d->url);

    if (isComponentComplete())
        load();
}

void QQuickImageBase::setSourceSize(const QSize& size)
{
    Q_D(QQuickImageBase);
    if (d->sourcesize == size)
        return;

    d->sourcesize = size;
    emit sourceSizeChanged();
    if (isComponentComplete())
        load();
}

QSize QQuickImageBase::sourceSize() const
{
    Q_D(const QQuickImageBase);

    int width = d->sourcesize.width();
    int height = d->sourcesize.height();
    return QSize(width != -1 ? width : d->pix.width(), height != -1 ? height : d->pix.height());
}

void QQuickImageBase::resetSourceSize()
{
    setSourceSize(QSize());
}

bool QQuickImageBase::cache() const
{
    Q_D(const QQuickImageBase);
    return d->cache;
}

void QQuickImageBase::setCache(bool cache)
{
    Q_D(QQuickImageBase);
    if (d->cache == cache)
        return;

    d->cache = cache;
    emit cacheChanged();
    if (isComponentComplete())
        load();
}

QImage QQuickImageBase::image() const
{
    Q_D(const QQuickImageBase);
    return d->pix.toImage();
}

void QQuickImageBase::setMirror(bool mirror)
{
    Q_D(QQuickImageBase);
    if (mirror == d->mirror)
        return;

    d->mirror = mirror;

    if (isComponentComplete())
        update();

    emit mirrorChanged();
}

bool QQuickImageBase::mirror() const
{
    Q_D(const QQuickImageBase);
    return d->mirror;
}

void QQuickImageBase::load()
{
    Q_D(QQuickImageBase);

    if (d->url.isEmpty()) {
        //d->pix.clear(this);
        if (d->progress != 0.0) {
            d->progress = 0.0;
            emit progressChanged(d->progress);
        }
        pixmapChange();
        d->status = Null;
        emit statusChanged(d->status);

        if (sourceSize() != d->oldSourceSize) {
            d->oldSourceSize = sourceSize();
            emit sourceSizeChanged();
        }

    } else {

        const qreal targetDevicePixelRatio = (window() ? window()->devicePixelRatio() : qApp->devicePixelRatio());
        d->devicePixelRatio = 1.0;

        QUrl loadUrl = d->url;
        resolve2xLocalFile(d->url, targetDevicePixelRatio, &loadUrl, &d->devicePixelRatio);
       //TODO Need to support proper image paths
        //qDebug() << "filename: " << d->url.fileName();
        qDebug() << "filename: " <<  QQmlFile::urlToLocalFileOrQrc(d->url);//d->url.toString();
		d->pix = QPixmap( QQmlFile::urlToLocalFileOrQrc(d->url));//d->url.toLocalFile());
        setMimage(d->pix.toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied),this);

    }
}

void QQuickImageBase::requestFinished()
{
    Q_D(QQuickImageBase);
    pixmapChange();
    emit statusChanged(d->status);
    if (sourceSize() != d->oldSourceSize) {
        d->oldSourceSize = sourceSize();
        emit sourceSizeChanged();
    }
    update();
}

void QQuickImageBase::requestProgress(qint64 received, qint64 total)
{
    Q_D(QQuickImageBase);
    if (d->status == Loading && total > 0) {
        d->progress = qreal(received)/total;
        emit progressChanged(d->progress);
    }
}

void QQuickImageBase::handleWindowChanged(QQuickWindow* window)
{
    if (window)
        connect(window, SIGNAL(screenChanged(QScreen*)), this, SLOT(handleScreenChanged(QScreen*)));
}

void QQuickImageBase::handleScreenChanged(QScreen* screen)
{
    // Screen DPI might have changed, reload images on screen change.
    if (screen && isComponentComplete())
        load();
}

void QQuickImageBase::componentComplete()
{
    Q_D(QQuickImageBase);
    QQuickItem::componentComplete();
    if (d->url.isValid())
        load();
}

void QQuickImageBase::pixmapChange()
{
    Q_D(QQuickImageBase);
    setImplicitSize(d->pix.width() / d->devicePixelRatio, d->pix.height() / d->devicePixelRatio);
}

// /path/to/foo.png -> path/too/foo@2x.png
static QString image2xPath(const QString &path)
{
    const int dotIndex = path.lastIndexOf(QLatin1Char('.'));
    if (dotIndex == -1)
        return path;
    if (path.contains(QLatin1String("@2x.")))
        return path;

    QString retinaPath = path;
    retinaPath.insert(dotIndex, QStringLiteral("@2x"));
    return retinaPath;
}

void QQuickImageBase::resolve2xLocalFile(const QUrl &url, qreal targetDevicePixelRatio, QUrl *sourceUrl, qreal *sourceDevicePixelRatio)
{
    Q_ASSERT(sourceUrl);
    Q_ASSERT(sourceDevicePixelRatio);

    // Bail out if "@2x" image loading is disabled, don't change the source url or devicePixelRatio.
    static bool disable2xImageLoading = !qgetenv("QT_HIGHDPI_DISABLE_2X_IMAGE_LOADING").isEmpty();
    if (disable2xImageLoading)
        return;

    QString localFile = QQmlFile::urlToLocalFileOrQrc(url);

    // Non-local file path: @2x loading is not supported.
    if (localFile.isEmpty())
        return;

    // Special case: the url in the QML source refers directly to an "@2x" file.
    if (localFile.contains(QLatin1String("@2x"))) {
        *sourceDevicePixelRatio = qreal(2.0);
        return;
    }

    // Don't load @2x files non normal-dpi displays.
    if (!(targetDevicePixelRatio > qreal(1.0)))
        return;

    // Look for an @2x version
    QString localFile2x = image2xPath(localFile);
    if (!QFile(localFile2x).exists())
        return;

    // @2x file found found: Change url and devicePixelRatio
    *sourceUrl = QUrl::fromLocalFile(localFile2x);
    *sourceDevicePixelRatio = qreal(2.0);
}

QT_END_NAMESPACE
