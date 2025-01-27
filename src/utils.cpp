//
// Created by benichn on 12/07/24.
//

#include "utils.h"
#include <xxhash.h>

#include "app.h"

vector<uchar> exec(const char* cmd)
{
    vector<uchar> result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe)
        throw runtime_error("popen() failed!");

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
    return roundedRect(x + r, y + r, w - 2 * r, h - 2 * r, r);
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
    return {cos(angle), sin(angle)};
}

QPoint radialVector(const float angle, const float radius)
{
    return (radius * unitVector(angle)).toPoint();
}

void drawArrow(QPainter* painter, const QPoint& pos, const float angle, const float radius, const float amplitude)
{
    const QPoint pts[] = {
        pos + radialVector(angle + M_PI - amplitude, radius),
        pos,
        pos + radialVector(angle + M_PI + amplitude, radius)
    };
    painter->drawPolyline(pts, 3);
}

void drawText(QPainter& painter, qreal x, qreal y, Qt::Alignment flags, const QString& text, QRectF* boundingRect)
{
    constexpr qreal size = 32767.0;
    QPointF corner(x, y - size);
    if (flags & Qt::AlignHCenter) corner.rx() -= size / 2.0;
    else if (flags & Qt::AlignRight) corner.rx() -= size;
    if (flags & Qt::AlignVCenter) corner.ry() += size / 2.0;
    else if (flags & Qt::AlignTop) corner.ry() += size;
    else flags |= Qt::AlignBottom;
    const QRectF rect{corner.x(), corner.y(), size, size};
    painter.drawText(rect, flags, text, boundingRect);
}

void drawText(QPainter& painter, const QPointF& point, Qt::Alignment flags, const QString& text, QRectF* boundingRect)
{
    drawText(painter, point.x(), point.y(), flags, text, boundingRect);
}

Task<> delay(const int ms)
{
    return QCoro::sleepFor(std::chrono::milliseconds(ms));
}

string calculateXXH3_64(const string& filePath)
{
    XXH3_state_t* state = XXH3_createState();
    if (state == nullptr)
    {
        throw runtime_error("Failed to create XXH3 state");
    }

    if (XXH3_64bits_reset(state) == XXH_ERROR)
    {
        XXH3_freeState(state);
        throw runtime_error("Failed to reset XXH3 state");
    }

    ifstream file(filePath, ifstream::binary);
    if (!file)
    {
        XXH3_freeState(state);
        throw runtime_error("Could not open file: " + filePath);
    }

    constexpr int bufSize = 4096;
    char buffer[bufSize];
    while (file.good())
    {
        file.read(buffer, bufSize);
        if (XXH3_64bits_update(state, buffer, file.gcount()) == XXH_ERROR)
        {
            XXH3_freeState(state);
            throw runtime_error("Failed to update XXH3 state");
        }
    }

    auto hash = XXH3_64bits_digest(state);
    XXH3_freeState(state);

    ostringstream hashString;
    hashString << std::hex << std::setw(16) << std::setfill('0') << hash;

    return hashString.str();
}

std::istream& ignoreUntil(std::istream& stream, const char delimiter)
{
    return stream.ignore(std::numeric_limits<std::streamsize>::max(), delimiter);
}

QPainterPath roundRect(const QRect& rect, const float radius)
{
    QPainterPath res;
    res.addRoundedRect(rect, radius, radius);
    return res;
}

int uniqueSelectedId(const QItemSelectionModel* model)
{
    auto idxs = model->selectedIndexes();
    return idxs.size() == 1 ? app().book().ids().at(idxs[0].row()) : -1;
}

// void updateKeys(json& src, const json& repl, const bool updateNulls)
// {
//     for (auto& kv : repl.items())
//     {
//         if (kv.value().is_null() && !updateNulls) continue;
//         const auto it = src.find(kv.key());
//         if (it == src.end() || updateNulls && it.value().is_null())
//         {
//             if (src.is_array()) src[stoi(kv.key())] = kv.value();
//             else src[kv.key()] = kv.value();
//         }
//         else if (jsonIsContainer(kv.value()))
//         {
//             updateNewKeys(src[kv.key()], kv.value());
//         }
//     }
// }

void updateNewKeys(json& src, const json& repl, const bool updateNulls)
{
    for (auto& kv : repl.items())
    {
        const auto it = src.find(kv.key());
        if (it == src.end() || updateNulls && it.value().is_null())
        {
            if (src.is_array()) src[stoi(kv.key())] = kv.value();
            else src[kv.key()] = kv.value();
        }
        else if (jsonIsContainer(kv.value()))
        {
            updateNewKeys(src[kv.key()], kv.value());
        }
    }
}

void nullifyKeys(json& src, const json& repl)
{
    for (auto& kv : src.items())
    {
        if (jsonIsContainer(kv.value()))
        {
            const auto it = repl.find(kv.key());
            nullifyKeys(kv.value(), it == repl.end() || it.value().is_null() ? json() : it.value());
        }
        else
        {
            const auto it = repl.find(kv.key());
            if (it == repl.end() || it.value().is_null())
            {
                if (src.is_array()) src[stoi(kv.key())] = nullptr;
                else src[kv.key()] = nullptr;
            }
        }
    }
}

bool jsonIsContainer(const json& j)
{
    return j.is_array() || j.is_object();
}

void updateExceptNulls(json& src, const json& repl)
{
    for (const auto& kv : repl.items())
    {
        const auto it = src.find(kv.key());
        if (!kv.value().is_null() && it != src.end())
        {
            if (jsonIsContainer(kv.value()))
            {
                updateExceptNulls(src[kv.key()], kv.value());
            }
            else
            {
                src[kv.key()] = kv.value();
            }
        }
    }
}

generator<QLayoutItem*> recursiveLayoutChildren(QLayout* layout)
{
    for (int i = 0; i < layout->count(); ++i)
    {
        const auto item = layout->itemAt(i);
        if (item->layout())
        {
            for (const auto it : recursiveLayoutChildren(item->layout()))
            {
                co_yield it;
            }
        }
        else
        {
            co_yield item;
        }
    }
}

const char* notation[] = {"i", "iv", "v", "ix", "x", "xl", "l", "xc", "c", "cd", "d", "cm", "m"};
const int numbers[] = {1, 4, 5, 9, 10, 40, 50, 90, 100, 400, 500, 900, 1000};
constexpr auto nbSize = arraySize(numbers);

string intToRoman(int num)
{
    ostringstream res;
    auto i = nbSize - 1;
    while (num > 0)
    {
        const auto n = num / numbers[i];
        for (int j = 0; j < n; ++j)
        {
            res << notation[i];
        }
        num -= n * numbers[i];
        --i;
    }
    return res.str();
}
