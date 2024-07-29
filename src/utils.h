//
// Created by benichn on 12/07/24.
//

#ifndef UTILS_H
#define UTILS_H

#include "qtimports.h"

vector<uchar> exec(const char* cmd);

int sgn(int x);

float ceilAbs(float x);
int ceilDiv(int x, int b);

int readIntBE(const uchar* bytes, int offset);
int readIntLE(const uchar* bytes, int offset);

QSize minSize(const QSize& size1, const QSize& size2);
QSize maxSize(const QSize& size1, const QSize& size2);

void offsetWidget(QWidget* widget, const QPoint& offset);

uchar bitRange(int l, int r, bool value);

int sq(int x);

int l22(const QPoint& p, const QPoint& q);

QRegion roundedRect(int x, int y, int w, int h, int r);

int cx(const QRect& rect);
int cy(const QRect& rect);

QRect mapRectTo(const QWidget* src, const QWidget* target, const QRect& rect);

QPointF unitVector(float angle);
QPoint radialVector(float angle, float radius);

void drawArrow(QPainter* painter, const QPoint& pos, float angle, float radius, float amplitude = M_PI/4);

#endif //UTILS_H
