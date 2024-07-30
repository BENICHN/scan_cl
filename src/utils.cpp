//
// Created by benichn on 12/07/24.
//

#include "utils.h"

#include <QCoro/QCoroTimer>

vector<uchar> exec(const char* cmd)
{
    vector<uchar> result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
        throw std::runtime_error("popen() failed!");

    constexpr size_t bufferSize = 1024;
    uchar buffer[bufferSize];

    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, bufferSize, pipe)) > 0)
    {
        result.insert(result.end(), buffer, buffer + bytesRead);
    }
    pclose(pipe);

    return result;
}

int sgn(const int x)
{
    return x == 0 ? 0 : x > 0 ? 1 : -1;
}

float ceilAbs(const float x)
{
    return x < 0 ? floor(x) : ceil(x);
}

int ceilDiv(const int x, const int b)
{
    return x / b + sgn(x % b);
}

int readIntBE(const uchar* bytes, const int offset)
{
    return bytes[offset] << 24 |
        bytes[offset + 1] << 16 |
        bytes[offset + 2] << 8 |
        bytes[offset + 3];
}

int readIntLE(const uchar* bytes, const int offset)
{
    return bytes[offset + 3] << 24 |
        bytes[offset + 2] << 16 |
        bytes[offset + 1] << 8 |
        bytes[offset];
}

QSize minSize(const QSize& size1, const QSize& size2)
{
    int minWidth = min(size1.width(), size2.width());
    int minHeight = min(size1.height(), size2.height());
    return {minWidth, minHeight};
}

QSize maxSize(const QSize& size1, const QSize& size2)
{
    int maxWidth = max(size1.width(), size2.width());
    int maxHeight = max(size1.height(), size2.height());
    return {maxWidth, maxHeight};
}

void offsetWidget(QWidget* widget, const QPoint& offset)
{
    widget->move(widget->pos() + offset);
}

uchar bitRange(const int l, const int r, const bool value)
{
    const uchar s1 = (1 << (r - l));
    const uchar s2 = s1 - 1;
    const uchar ones = s2 << 8 - r;
    return value ? ones : ~ones;
}

int sq(const int x)
{
    return x * x;
}

int l22(const QPoint& p, const QPoint& q)
{
    return sq(p.x() - q.x()) + sq(p.y() - q.y());
}

QRegion roundedRect(const int x, const int y, const int w, const int h, const int r)
{
    return QRegion(x, y, w, h, QRegion::Rectangle) + QRegion(x - r, y - r, 2 * r, 2 * r, QRegion::Ellipse) +
        QRegion(x - r, y + h - r, 2 * r, 2 * r, QRegion::Ellipse) +
        QRegion(x + w - r, y + h - r, 2 * r, 2 * r, QRegion::Ellipse) +
        QRegion(x + w - r, y - r, 2 * r, 2 * r, QRegion::Ellipse) + QRegion(x - r, y, r, h, QRegion::Rectangle) +
        QRegion(x, y - r, w, r, QRegion::Rectangle) + QRegion(x + w, y, r, h, QRegion::Rectangle) + QRegion(
            x, y + h, w, r, QRegion::Rectangle);
}

QRegion roundedRectInner(int x, int y, int w, int h, int r)
{
    return roundedRect(x+r, y+r, w-2*r, h-2*r, r);
}

int cx(const QRect& rect)
{
    return .5 * (rect.left() + rect.right());
}

int cy(const QRect& rect)
{
    return .5 * (rect.top() + rect.bottom());
}

QRect mapRectTo(const QWidget* src, const QWidget* target, const QRect& rect)
{
    return {
        src->mapTo(target, rect.topLeft()),
        rect.size()
    };
}

QPointF unitVector(const float angle)
{
    return { cos(angle), sin(angle) };
}

QPoint radialVector(const float angle, const float radius)
{
    return (radius * unitVector(angle)).toPoint();
}

void drawArrow(QPainter* painter, const QPoint& pos, const float angle, const float radius, const float amplitude)
{
    const QPoint pts[] = {
        pos + radialVector(angle+M_PI-amplitude, radius),
        pos,
        pos + radialVector(angle+M_PI+amplitude, radius)
    };
    painter->drawPolyline(pts, 3);
}

void drawText(QPainter& painter, qreal x, qreal y, Qt::Alignment flags, const QString& text, QRectF* boundingRect)
{
    const qreal size = 32767.0;
    QPointF corner(x, y - size);
    if (flags & Qt::AlignHCenter) corner.rx() -= size/2.0;
    else if (flags & Qt::AlignRight) corner.rx() -= size;
    if (flags & Qt::AlignVCenter) corner.ry() += size/2.0;
    else if (flags & Qt::AlignTop) corner.ry() += size;
    else flags |= Qt::AlignBottom;
    QRectF rect{corner.x(), corner.y(), size, size};
    painter.drawText(rect, flags, text, boundingRect);
}

void drawText(QPainter& painter, const QPointF& point, Qt::Alignment flags, const QString& text, QRectF* boundingRect)
{
    drawText(painter, point.x(), point.y(), flags, text, boundingRect);
}

QCoro::Task<> delay(const int ms)
{
    return QCoro::sleepFor(std::chrono::milliseconds(ms));
}
