//
// Created by benichn on 09/08/24.
//

#ifndef OPENCVIMPORTS_H
#define OPENCVIMPORTS_H

#include <opencv2/opencv.hpp>

using cv::Mat;
using cv::imread;
using cv::imwrite;
using cv::cvtColor;
using cv::Scalar;
using cv::connectedComponentsWithStats;

struct CVType
{
    uchar depth;
    uchar chanels;

    CVType(uchar depth, uchar chanels);
    CVType(int type);
};

struct CCStats
{
    Mat labels;
    Mat stats;
    Mat centroids;
};

CCStats connectedComponentsWithStats(const Mat& src);

Mat isIn(const Mat& labels, const std::vector<int>& elements);

Mat surroundWith(const Mat& src, int t, int l, int b, int r, const Scalar& value);

#endif //OPENCVIMPORTS_H
