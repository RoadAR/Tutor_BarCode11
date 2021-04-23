//
//  bar_code.cpp
//  BarCode11
//
//  Created by Alexander Graschenkov on 19.04.2021.
//

#include "bar_code.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

#define DEBUG_CODE_DRAW

#ifdef DEBUG_CODE_DRAW
#define debug_imshow(_a_, _b_) imshow(_a_, _b_)
#else
#define debug_imshow(_a_, _b_)
#endif

namespace preprocess {

void runSobel(const Mat &src_gray, cv::Mat &dstGray, cv::Mat *angles, bool normalize) {
    int scale = 1;
    int delta = 0;
    int ddepth = CV_32F;
    
    Mat grad_x, grad_y;
    Mat abs_grad_x, abs_grad_y;
    
    Sobel(src_gray, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT);
    Sobel(src_gray, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT);
    
    /// Total Gradient (approximate)
    if (angles) {
        phase(grad_x, grad_y, *angles, true);
    }
    
    if (normalize) {
        Mat temp;
        magnitude(grad_x, grad_y, temp);
        convertScaleAbs( temp, dstGray );
    } else {
        magnitude(grad_x, grad_y, dstGray);
//        addWeighted( grad_x, 0.5, grad_y, 0.5, 0, dstGray );
    }
}

void runErodeDilate(const cv::Mat &img, cv::Mat &dst) {
    Mat element = getStructuringElement( MORPH_ELLIPSE, Size(7, 7), Point(3, 3));
    dilate(img, dst, element);
    
    erode(dst, dst, element, Point(-1, -1), 3);
}


cv::Rect getRectConnectedComponent(const cv::Mat &statRow) {
    return Rect(statRow.at<int>(cv::CC_STAT_LEFT),
                statRow.at<int>(cv::CC_STAT_TOP),
                statRow.at<int>(cv::CC_STAT_WIDTH),
                statRow.at<int>(cv::CC_STAT_HEIGHT));
}

struct BlobInfo {
    cv::Rect r;
    int pixelsCount = 0;
    cv::Mat img;
};

BlobInfo findMaxBlob(const cv::Mat &frame) {
    cv::Mat labels, centroids, stats;
    int nccomps = cv::connectedComponentsWithStats(frame, labels, stats, centroids);
    
    
    int maxAreaIdx = -1;
    int maxArea = 0;
    for (int i = 1; i < nccomps; i++) {
        int area = stats.row(i).at<int>(cv::CC_STAT_AREA);
        if (area > maxArea) {
            maxArea = area;
            maxAreaIdx = i;
        }
    }
    if (maxAreaIdx < 0) {
        return {};
    }
    
    BlobInfo info;
    info.r = getRectConnectedComponent(stats.row(maxAreaIdx));
    info.pixelsCount = maxArea;
    info.img = Mat(frame.size(), CV_8U, Scalar(0));
    for (int i = 0; i < labels.total(); i++) {
        info.img.at<uchar>(i) = (labels.at<int>(i) == maxAreaIdx) ? 255 : 0;
    }
    return info;
}

vector<Point> getContour(cv::Mat &img) {
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(img, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
    return contours.front();
}


void drawRotatedRect(cv::Mat &img, cv::RotatedRect r, Scalar color, int lineWidth) {
    Point2f rectPoints[4]; r.points(rectPoints);
    
    if (lineWidth > 0) {
        for(int j = 0; j < 4; j++) {
            line(img, rectPoints[j], rectPoints[(j+1)%4], color, lineWidth);
        }
    } else {
        Point points[4];
        for (int i = 0; i < 4; i++) {
            points[i] = rectPoints[i];
        }
        fillConvexPoly(img, points, 4, color);
    }
}

Mat getAffineTransformForRotatedRect(RotatedRect rr) {
    float angle = rr.angle * M_PI / 180.0;
    float sinA = sin(angle), cosA = cos(angle);
    float data[6] = {
        cosA, sinA, rr.size.width/2.0f - cosA * rr.center.x - sinA * rr.center.y,
        -sinA, cosA, rr.size.height/2.0f - cosA * rr.center.y + sinA * rr.center.x};
    Mat rot_mat(2, 3, CV_32FC1, data);
    return rot_mat.clone();
}

Mat getRotatedRectImg(const cv::Mat &mat, RotatedRect rr, cv::Size forceSize) {
    Mat M, result;
    M = getAffineTransformForRotatedRect(rr);
    
    Size toSize = rr.size;
    if (forceSize.area() > 0) {
        toSize = forceSize;
        M.row(0) *= forceSize.width / rr.size.width;
        M.row(1) *= forceSize.height / rr.size.height;
    }
    warpAffine(mat, result, M, toSize, INTER_CUBIC);
    
    return result;
}

// https://en.wikipedia.org/wiki/Run-length_encoding
vector<int> image2rle(const cv::Mat &img) {
    int bw[2] = {0, 0};
    vector<int> pattern;
    bool fillWhite = false;
    for (int c = 0; c < img.cols; c++) {
        bool isWhite = img.at<uchar>(c) > 100;
        if (fillWhite != isWhite) {
            fillWhite = !fillWhite;
            if (fillWhite == false) {
                pattern.push_back(-bw[0]);
                pattern.push_back(bw[1]);
                bw[0] = 0; bw[1] = 1;
            }
        }
        bw[isWhite]++;
    }
    if (bw[0] > 0) {
        pattern.push_back(-bw[0]);
        pattern.push_back(bw[1]);
    }
    return pattern;
}
}

using namespace preprocess;

BarCode::BarCode(const std::vector<std::shared_ptr<CodeDecoder>> &decoders): decoders_(decoders) {
    
}

BarCode::Recognition BarCode::detectAndRecognize(cv::Mat &image) {
    if (image.channels() == 1) {
        gray_ = image;
    } else {
        cvtColor(image, gray_, COLOR_BGR2GRAY);
    }
    
    // Image preprocessing
    Mat procImg;
    runSobel(gray_, procImg, nullptr, true);
    debug_imshow("sobel", procImg);
    
    runErodeDilate(procImg, procImg);
    
    debug_imshow("morph", procImg);
    threshold(procImg, procImg, 200, 255, THRESH_BINARY);
    debug_imshow("thresh", procImg);
    
    auto blobInfo = findMaxBlob(procImg);
    debug_imshow("component", blobInfo.img);
    
    
    Mat cropImg = blobInfo.img(blobInfo.r);
    vector<Point> contour = getContour(cropImg);
    RotatedRect rr = minAreaRect(contour);
    rr.center += (Point2f)blobInfo.r.tl();
    if (rr.size.width < rr.size.height) {
        swap(rr.size.width, rr.size.height);
        rr.angle += 90;
    }
    
    
#ifdef DEBUG_CODE_DRAW
    Mat debugImg;
    cvtColor(gray_, debugImg, COLOR_GRAY2BGR);
    rectangle(debugImg, blobInfo.r, CV_RGB(255, 0, 0), 5);
    drawRotatedRect(debugImg, rr, CV_RGB(0, 255, 0), 3);
    imshow("debug", debugImg);
#endif
    
    // Crop image
    RotatedRect cropRect = rr;
    cropRect.size.height = 11;
    cropRect.size.width *= 1.1;
    
    Size outSize = rr.size;
    outSize.height = 1;
    outSize.width *= 2;
    Mat code1PixImg = getRotatedRectImg(gray_, cropRect, outSize);
    
    threshold(code1PixImg, code1PixImg, 0, 255, THRESH_OTSU);
    Mat debugCodeImg;
    resize(code1PixImg, debugCodeImg, Size(outSize.width, 100));
    imshow("cropped img", debugCodeImg);
    
    // Read image
    vector<int> pattern = image2rle(code1PixImg);
    
#ifdef DEBUG_CODE_DRAW
    cout << "Pattern> ";
    for (auto val : pattern) {
        cout << val << " ";
    }
    cout << endl;
#endif
    
    // Decoding
    BarCode::Recognition result;
    for (const auto &decoder : decoders_) {
        string resultStr = decoder->decode(pattern);
        if (!resultStr.empty()) {
            result.value = resultStr;
            result.name = decoder->getName();
            result.location = rr;
            break;
        }
    }
    return result;
}
