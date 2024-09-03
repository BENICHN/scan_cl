//
// Created by benichn on 12/07/24.
//

#ifndef UTILS_H
#define UTILS_H

#include <qlayout.h>
#include <qlayoutitem.h>

#include "imports/stdimports.h"
#include "imports/qtimports.h"
#include "imports/qcoroimports.h"
#include "imports/jsonimports.h"

vector<uchar> exec(const char* cmd);

constexpr int sgn(const int x)
{
    return x == 0 ? 0 : x > 0 ? 1 : -1;
}

constexpr float ceilAbs(const float x)
{
    return x < 0 ? floorf(x) : ceilf(x);
}

constexpr int ceilDiv(const int x, const int b)
{
    return x / b + sgn(x % b);
}

int readIntBE(const uchar* bytes, int offset);
int readIntLE(const uchar* bytes, int offset);

QSize minSize(const QSize& size1, const QSize& size2);
QSize maxSize(const QSize& size1, const QSize& size2);

void offsetWidget(QWidget* widget, const QPoint& offset);

uchar bitRange(int l, int r, bool value);

int sq(int x);

int l22(const QPoint& p, const QPoint& q);

QRegion roundedRect(int x, int y, int w, int h, int r);
QRegion roundedRectInner(int x, int y, int w, int h, int r);

int cx(const QRect& rect);
int cy(const QRect& rect);

QRect mapRectTo(const QWidget* src, const QWidget* target, const QRect& rect);

QPointF unitVector(float angle);
QPoint radialVector(float angle, float radius);

void drawArrow(QPainter* painter, const QPoint& pos, float angle, float radius, float amplitude = M_PI / 4);

void drawText(QPainter& painter, qreal x, qreal y, Qt::Alignment flags,
              const QString& text, QRectF* boundingRect = nullptr);

void drawText(QPainter& painter, const QPointF& point, Qt::Alignment flags,
              const QString& text, QRectF* boundingRect = nullptr);

Task<> delay(int ms);

template <template<class,class,class...> class C, typename K, typename V, typename... Args>
V mapAtDef(const C<K, V, Args...>& m, K const& key, const V& defval)
{
    typename C<K, V, Args...>::const_iterator it = m.find(key);
    if (it == m.end())
        return defval;
    return it->second;
}

template <template<class,class,class...> class C, typename K, typename V, typename... Args>
V* mapAtNull(const C<K, V, Args...>& m, K const& key)
{
    typename C<K, V, Args...>::const_iterator it = m.find(key);
    if (it == m.end())
        return nullptr;
    return &it->second;
}

string calculateXXH3_64(const string& filePath);

std::istream& ignoreUntil(std::istream& stream, char delimiter);

template <typename T, typename K>
optional<T> atOpt(const json& j, K&& key)
{
    const auto it = j.find(std::forward<K>(key));
    if (it == j.end()) return nullopt;
    return {it.value()};
}

constexpr QRect fitIn(const QRect& src, const QRect& dest)
{
    const int w = src.width();
    const int h = src.height();
    const int W = dest.width();
    const int H = dest.height();

    const float ws = static_cast<float>(W) / w;
    const float hs = static_cast<float>(H) / h;
    if (ws < hs)
    {
        const int nh = h * ws;
        return {dest.left(), dest.top() + (H - nh) / 2, W, nh};
    }
    else
    {
        const int nw = w * hs;
        return {dest.left() + (W - nw) / 2, dest.top(), nw, H};
    }
}

QPainterPath roundRect(const QRect& rect, float radius);

int uniqueSelectedId(const QItemSelectionModel* model);

void updateNewKeys(json& src, const json& repl, bool updateNulls = true);
void nullifyKeys(json& src, const json& repl);
bool jsonIsContainer(const json& j);

generator<QLayoutItem*> recursiveLayoutChildren(QLayout* layout);

template <typename T, size_t N>
constexpr size_t arraySize(T (&)[N]) { return N; }

string intToRoman(int num);

#endif //UTILS_H
