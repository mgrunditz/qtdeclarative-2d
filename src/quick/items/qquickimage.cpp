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

#include "qquickimage_p.h"
#include "qquickimage_p_p.h"


#include <QtQuick/private/qsgcontext_p.h>

#include <QtGui/qpainter.h>
#include <qmath.h>
#include <QBackingStore>
QT_BEGIN_NAMESPACE
//#include "qquickimage.moc"

QQuickImagePrivate::QQuickImagePrivate()
    : fillMode(QQuickImage::Stretch)
    , paintedWidth(0)
    , paintedHeight(0)
    , pixmapChanged(false)
    , mipmap(false)
    , hAlign(QQuickImage::AlignHCenter)
    , vAlign(QQuickImage::AlignVCenter)
{
}

/*!
    \qmltype Image
    \instantiates QQuickImage
    \inqmlmodule QtQuick
    \ingroup qtquick-visual
    \inherits Item
    \brief Displays an image

    The Image type displays an image.

    The source of the image is specified as a URL using the \l source property.
    Images can be supplied in any of the standard image formats supported by Qt,
    including bitmap formats such as PNG and JPEG, and vector graphics formats
    such as SVG. If you need to display animated images, use \l AnimatedSprite
    or \l AnimatedImage.

    If the \l{Item::width}{width} and \l{Item::height}{height} properties are not
    specified, the Image automatically uses the size of the loaded image.
    By default, specifying the width and height of the item causes the image
    to be scaled to that size. This behavior can be changed by setting the
    \l fillMode property, allowing the image to be stretched and tiled instead.

    \section1 Example Usage

    The following example shows the simplest usage of the Image type.

    \snippet qml/image.qml document

    \beginfloatleft
    \image declarative-qtlogo.png
    \endfloat

    \clearfloat

    \section1 Performance

    By default, locally available images are loaded immediately, and the user interface
    is blocked until loading is complete. If a large image is to be loaded, it may be
    preferable to load the image in a low priority thread, by enabling the \l asynchronous
    property.

    If the image is obtained from a network rather than a local resource, it is
    automatically loaded asynchronously, and the \l progress and \l status properties
    are updated as appropriate.

    Images are cached and shared internally, so if several Image items have the same \l source,
    only one copy of the image will be loaded.

    \b Note: Images are often the greatest user of memory in QML user interfaces.  It is recommended
    that images which do not form part of the user interface have their
    size bounded via the \l sourceSize property. This is especially important for content
    that is loaded from external sources or provided by the user.

    \sa {Qt Quick Examples - Image Elements}, QQuickImageProvider
*/

QQuickImage::QQuickImage(QQuickItem *parent)
    : QQuickImageBase(*(new QQuickImagePrivate), parent)
{
}

QQuickImage::QQuickImage(QQuickImagePrivate &dd, QQuickItem *parent)
    : QQuickImageBase(dd, parent)
{
    //TRY this
    load();
}

QQuickImage::~QQuickImage()
{
    Q_D(QQuickImage);
    //TODO fixme
}

void QQuickImagePrivate::setImage(const QImage &image)
{
    Q_Q(QQuickImage);
    pix = QPixmap::fromImage(image);
    q->pixmapChange();
    status = pix.isNull() ? QQuickImageBase::Null : QQuickImageBase::Ready;

    q->update();
}

/*!
    \qmlproperty enumeration QtQuick::Image::fillMode

    Set this property to define what happens when the source image has a different size
    than the item.

    \list
    \li Image.Stretch - the image is scaled to fit
    \li Image.PreserveAspectFit - the image is scaled uniformly to fit without cropping
    \li Image.PreserveAspectCrop - the image is scaled uniformly to fill, cropping if necessary
    \li Image.Tile - the image is duplicated horizontally and vertically
    \li Image.TileVertically - the image is stretched horizontally and tiled vertically
    \li Image.TileHorizontally - the image is stretched vertically and tiled horizontally
    \li Image.Pad - the image is not transformed
    \endlist

    \table

    \row
    \li \image declarative-qtlogo-stretch.png
    \li Stretch (default)
    \qml
    Image {
        width: 130; height: 100
        source: "qtlogo.png"
    }
    \endqml

    \row
    \li \image declarative-qtlogo-preserveaspectfit.png
    \li PreserveAspectFit
    \qml
    Image {
        width: 130; height: 100
        fillMode: Image.PreserveAspectFit
        source: "qtlogo.png"
    }
    \endqml

    \row
    \li \image declarative-qtlogo-preserveaspectcrop.png
    \li PreserveAspectCrop
    \qml
    Image {
        width: 130; height: 100
        fillMode: Image.PreserveAspectCrop
        source: "qtlogo.png"
        clip: true
    }
    \endqml

    \row
    \li \image declarative-qtlogo-tile.png
    \li Tile
    \qml
    Image {
        width: 120; height: 120
        fillMode: Image.Tile
        horizontalAlignment: Image.AlignLeft
        verticalAlignment: Image.AlignTop
        source: "qtlogo.png"
    }
    \endqml

    \row
    \li \image declarative-qtlogo-tilevertically.png
    \li TileVertically
    \qml
    Image {
        width: 120; height: 120
        fillMode: Image.TileVertically
        verticalAlignment: Image.AlignTop
        source: "qtlogo.png"
    }
    \endqml

    \row
    \li \image declarative-qtlogo-tilehorizontally.png
    \li TileHorizontally
    \qml
    Image {
        width: 120; height: 120
        fillMode: Image.TileHorizontally
        verticalAlignment: Image.AlignLeft
        source: "qtlogo.png"
    }
    \endqml

    \endtable

    Note that \c clip is \c false by default which means that the item might
    paint outside its bounding rectangle even if the fillMode is set to \c PreserveAspectCrop.

    \sa {Qt Quick Examples - Image Elements}
*/

/*int QQuickImage::border() const
{
	return (0);
}
*/
void QQuickImage::setborder( int b )
{

}
QQuickImage::FillMode QQuickImage::fillMode() const
{
    Q_D(const QQuickImage);
    return d->fillMode;
}

void QQuickImage::setFillMode(FillMode mode)
{
    Q_D(QQuickImage);
    if (d->fillMode == mode)
        return;
    d->fillMode = mode;
    update();
    updatePaintedGeometry();
    emit fillModeChanged();
}

/*!

    \qmlproperty real QtQuick::Image::paintedWidth
    \qmlproperty real QtQuick::Image::paintedHeight

    These properties hold the size of the image that is actually painted.
    In most cases it is the same as \c width and \c height, but when using a
    \c fillMode \c PreserveAspectFit or \c fillMode \c PreserveAspectCrop
    \c paintedWidth or \c paintedHeight can be smaller or larger than
    \c width and \c height of the Image item.
*/
qreal QQuickImage::paintedWidth() const
{
    Q_D(const QQuickImage);
    return d->paintedWidth;
}

qreal QQuickImage::paintedHeight() const
{
    Q_D(const QQuickImage);
    return d->paintedHeight;
}

/*!
    \qmlproperty enumeration QtQuick::Image::status

    This property holds the status of image loading.  It can be one of:
    \list
    \li Image.Null - no image has been set
    \li Image.Ready - the image has been loaded
    \li Image.Loading - the image is currently being loaded
    \li Image.Error - an error occurred while loading the image
    \endlist

    Use this status to provide an update or respond to the status change in some way.
    For example, you could:

    \list
    \li Trigger a state change:
    \qml
        State { name: 'loaded'; when: image.status == Image.Ready }
    \endqml

    \li Implement an \c onStatusChanged signal handler:
    \qml
        Image {
            id: image
            onStatusChanged: if (image.status == Image.Ready) console.log('Loaded')
        }
    \endqml

    \li Bind to the status value:
    \qml
        Text { text: image.status == Image.Ready ? 'Loaded' : 'Not loaded' }
    \endqml
    \endlist

    \sa progress
*/

/*!
    \qmlproperty real QtQuick::Image::progress

    This property holds the progress of image loading, from 0.0 (nothing loaded)
    to 1.0 (finished).

    \sa status
*/

/*!
    \qmlproperty bool QtQuick::Image::smooth

    This property holds whether the image is smoothly filtered when scaled or
    transformed.  Smooth filtering gives better visual quality, but it may be slower
    on some hardware.  If the image is displayed at its natural size, this property has
    no visual or performance effect.

    By default, this property is set to true.

    \sa mipmap
*/

/*!
    \qmlproperty QSize QtQuick::Image::sourceSize

    This property holds the actual width and height of the loaded image.

    Unlike the \l {Item::}{width} and \l {Item::}{height} properties, which scale
    the painting of the image, this property sets the actual number of pixels
    stored for the loaded image so that large images do not use more
    memory than necessary. For example, this ensures the image in memory is no
    larger than 1024x1024 pixels, regardless of the Image's \l {Item::}{width} and
    \l {Item::}{height} values:

    \code
    Rectangle {
        width: ...
        height: ...

        Image {
           anchors.fill: parent
           source: "reallyBigImage.jpg"
           sourceSize.width: 1024
           sourceSize.height: 1024
        }
    }
    \endcode

    If the image's actual size is larger than the sourceSize, the image is scaled down.
    If only one dimension of the size is set to greater than 0, the
    other dimension is set in proportion to preserve the source image's aspect ratio.
    (The \l fillMode is independent of this.)

    If both the sourceSize.width and sourceSize.height are set the image will be scaled
    down to fit within the specified size, maintaining the image's aspect ratio.  The actual
    size of the image after scaling is available via \l Item::implicitWidth and \l Item::implicitHeight.

    If the source is an intrinsically scalable image (eg. SVG), this property
    determines the size of the loaded image regardless of intrinsic size.
    Avoid changing this property dynamically; rendering an SVG is \e slow compared
    to an image.

    If the source is a non-scalable image (eg. JPEG), the loaded image will
    be no greater than this property specifies. For some formats (currently only JPEG),
    the whole image will never actually be loaded into memory.

    sourceSize can be cleared to the natural size of the image
    by setting sourceSize to \c undefined.

    \note \e {Changing this property dynamically causes the image source to be reloaded,
    potentially even from the network, if it is not in the disk cache.}
*/

/*!
    \qmlproperty url QtQuick::Image::source

    Image can handle any image format supported by Qt, loaded from any URL scheme supported by Qt.

    The URL may be absolute, or relative to the URL of the component.

    \sa QQuickImageProvider
*/

/*!
    \qmlproperty bool QtQuick::Image::asynchronous

    Specifies that images on the local filesystem should be loaded
    asynchronously in a separate thread.  The default value is
    false, causing the user interface thread to block while the
    image is loaded.  Setting \a asynchronous to true is useful where
    maintaining a responsive user interface is more desirable
    than having images immediately visible.

    Note that this property is only valid for images read from the
    local filesystem.  Images loaded via a network resource (e.g. HTTP)
    are always loaded asynchronously.
*/

/*!
    \qmlproperty bool QtQuick::Image::cache

    Specifies whether the image should be cached. The default value is
    true. Setting \a cache to false is useful when dealing with large images,
    to make sure that they aren't cached at the expense of small 'ui element' images.
*/

/*!
    \qmlproperty bool QtQuick::Image::mirror

    This property holds whether the image should be horizontally inverted
    (effectively displaying a mirrored image).

    The default value is false.
*/

/*!
    \qmlproperty enumeration QtQuick::Image::horizontalAlignment
    \qmlproperty enumeration QtQuick::Image::verticalAlignment

    Sets the horizontal and vertical alignment of the image. By default, the image is center aligned.

    The valid values for \c horizontalAlignment are \c Image.AlignLeft, \c Image.AlignRight and \c Image.AlignHCenter.
    The valid values for \c verticalAlignment are \c Image.AlignTop, \c Image.AlignBottom
    and \c Image.AlignVCenter.
*/
void QQuickImage::updatePaintedGeometry()
{
    Q_D(QQuickImage);

    if (d->fillMode == PreserveAspectFit) {
        if (!d->pix.width() || !d->pix.height()) {
            setImplicitSize(0, 0);
            return;
        }
        qreal w = widthValid() ? width() : d->pix.width();
        qreal widthScale = w / qreal(d->pix.width());
        qreal h = heightValid() ? height() : d->pix.height();
        qreal heightScale = h / qreal(d->pix.height());
        if (widthScale <= heightScale) {
            d->paintedWidth = w;
            d->paintedHeight = widthScale * qreal(d->pix.height());
        } else if (heightScale < widthScale) {
            d->paintedWidth = heightScale * qreal(d->pix.width());
            d->paintedHeight = h;
        }
        qreal iHeight = (widthValid() && !heightValid()) ? d->paintedHeight : d->pix.height();
        qreal iWidth = (heightValid() && !widthValid()) ? d->paintedWidth : d->pix.width();
        setImplicitSize(iWidth, iHeight);

    } else if (d->fillMode == PreserveAspectCrop) {
        if (!d->pix.width() || !d->pix.height())
            return;
        qreal widthScale = width() / qreal(d->pix.width());
        qreal heightScale = height() / qreal(d->pix.height());
        if (widthScale < heightScale) {
            widthScale = heightScale;
        } else if (heightScale < widthScale) {
            heightScale = widthScale;
        }

        d->paintedHeight = heightScale * qreal(d->pix.height());
        d->paintedWidth = widthScale * qreal(d->pix.width());
    } else if (d->fillMode == Pad) {
        d->paintedWidth = d->pix.width();
        d->paintedHeight = d->pix.height();
    } else {
        d->paintedWidth = width();
        d->paintedHeight = height();
    }
    emit paintedGeometryChanged();
}
/*void QQuickImage::borderChanged()
{

}*/
void QQuickImage::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickImageBase::geometryChanged(newGeometry, oldGeometry);
    updatePaintedGeometry();
}

QRectF QQuickImage::boundingRect() const
{
    Q_D(const QQuickImage);
    return QRectF(0, 0, qMax(width(), d->paintedWidth), qMax(height(), d->paintedHeight));
}
void QQuickImage::updatePaintNode()
{
    Q_D(QQuickImage);
 if (d->pix.toImage().isNull())
    load();
    if (!d->pix.toImage().isNull())
    {
       //setMimage(d->pix.scaled(QSize(d->pix.width()*scale(),d->pix.height()*scale())).toImage(),this);
       //setMimage(d->pix.scaled(QSize(width(),height())).toImage(),this);
       setBufferSize(d->pix.scaled(QSize(width(),height())).toImage().width()*d->pix.scaled(QSize(width(),height())).toImage().height()*4,d->pix.scaled(QSize(width(),height())).toImage().width(),d->pix.scaled(QSize(width(),height())).toImage().height());
       setPixels((char*)d->pix.scaled(QSize(width(),height())).toImage().bits());
       if (width() <=0 || height() <=0)
       {
       if (scale())
       {
       setWidth(d->pix.scaled(QSize(d->pix.width()*scale(),d->pix.height()*scale())).width());
       setHeight(d->pix.scaled(QSize(d->pix.height()*scale(),d->pix.width()*scale())).height());
       } else {
       setWidth(d->pix.width());
       setHeight(d->pix.height());
       }
       }
}

    }

void QQuickImage::pixmapChange()
{
    Q_D(QQuickImage);
    // PreserveAspectFit calculates the implicit size differently so we
    // don't call our superclass pixmapChange(), since that would
    // result in the implicit size being set incorrectly, then updated
    // in updatePaintedGeometry()
    if (d->fillMode != PreserveAspectFit)
        QQuickImageBase::pixmapChange();
    updatePaintedGeometry();
    d->pixmapChanged = true;

    // When the pixmap changes, such as being deleted, we need to update the textures
    //update();
}

QQuickImage::VAlignment QQuickImage::verticalAlignment() const
{
    Q_D(const QQuickImage);
    return d->vAlign;
}

void QQuickImage::setVerticalAlignment(VAlignment align)
{
    Q_D(QQuickImage);
    if (d->vAlign == align)
        return;

    d->vAlign = align;
    update();
    updatePaintedGeometry();
    emit verticalAlignmentChanged(align);
}

QQuickImage::HAlignment QQuickImage::horizontalAlignment() const
{
    Q_D(const QQuickImage);
    return d->hAlign;
}

void QQuickImage::setHorizontalAlignment(HAlignment align)
{
    Q_D(QQuickImage);
    if (d->hAlign == align)
        return;

    d->hAlign = align;
    update();
    updatePaintedGeometry();
    emit horizontalAlignmentChanged(align);
}

/*!
    \qmlproperty bool QtQuick::Image::mipmap
    \since 5.3

    This property holds whether the image uses mipmap filtering when scaled or
    transformed.

    Mipmap filtering gives better visual quality when scaling down
    compared to smooth, but it may come at a performance cost (both when
    initializing the image and during rendering).

    By default, this property is set to false.

    \sa smooth
 */

bool QQuickImage::mipmap() const
{
    Q_D(const QQuickImage);
    return d->mipmap;
}

void QQuickImage::setMipmap(bool use)
{
    Q_D(QQuickImage);
    if (d->mipmap == use)
        return;
    d->mipmap = use;
    emit mipmapChanged(d->mipmap);

    d->pixmapChanged = true;
    update();
}

QT_END_NAMESPACE
