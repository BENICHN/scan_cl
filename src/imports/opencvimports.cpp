//
// Created by benichn on 09/08/24.
//

#include "stdimports.h"
#include "opencvimports.h"

CVType::CVType(const uchar depth, const uchar chanels): depth(depth),
                                                        chanels(chanels)
{
}

CVType::CVType(const int type): CVType(type & CV_MAT_DEPTH_MASK,1 + (type >> CV_CN_SHIFT))
{
}

CCStats connectedComponentsWithStats(const Mat& src)
{
    CCStats res;
    connectedComponentsWithStats(src, res.labels, res.stats, res.centroids);
    return res;
}

Mat isIn(const Mat& labels, const std::vector<int>& elements)
{
    assert(labels.depth() == CV_32S);
    cv::Mat1b res(labels.rows, labels.cols);
    const auto* src = reinterpret_cast<int*>(labels.data);
    auto* dst = res.data;
    for (int i = 0; i < labels.rows*labels.cols; ++i)
    {
        *dst = str::contains(elements, *src) ? 255 : 0;
        ++dst;
        ++src;
    }
    return res;
}
