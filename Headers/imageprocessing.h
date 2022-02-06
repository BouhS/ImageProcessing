#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <QMainWindow>
#include <QLabel>
#include <QScrollArea>
#include <QDialog>
#include <QFileDialog>
#include <QImage>
#include <QRgb>
#include <QList>

#include <thread>
#include <functional>
#include <string>

using namespace std;

class ImageProcessing
{

public:
    ImageProcessing(QImage *image = nullptr);
    ~ImageProcessing();

private:
    void createNewImage();

    void setAsCurrentImage(QImage *image);

public:
    QImage* convertToGrayScale(const  uchar* imageData, const int width, const int height, const QImage::Format format);
    // Blur
    QImage* meanBlur(const  uchar* imageData, const int width, const int height, const QImage::Format format);
    QImage* gaussianBlur3x3(const  uchar* imageData, const int width, const int height, const QImage::Format format);
    QImage* gaussianBlur5x5(const  uchar* imageData, const int width, const int height, const QImage::Format format);
    QImage* medianFilter(const uchar* imageData, const int width, const int height, QImage::Format format);
    // variation of intensity to maintain edges visible
    QImage* variationFilter(const uchar* imageData, const int width, const int height, QImage::Format format);
    // Histogram
    void computeHistogram(const QImage* image,std::vector<int> *redHistogram,std::vector<int> *greenHistogram,std::vector<int> *blueHistogram);
    static void fillHistogram(const QImage image,const int x_start,const int x_end, const int height, vector< vector<int> *> * histograms, const int i);

    //Edge detection
    QImage* gradientFilter(const  uchar* imageData,const int width, const int height,const QImage::Format format);
    QImage* horizontalSobelGradientFilter(const  uchar* imageData, const int width, const int height, const QImage::Format format);
    QImage* verticalSobelGradientFilter(const  uchar* imageData, const int width, const int height, const QImage::Format format);
    QImage* applyFilter(const uchar *image, const int width, const int height, const QImage::Format format, const int kernelRadius, const int kernel[], const float kernelParameter,
                                              QColor (*convolution)(const uchar *,const int, const int,
                                                                   const int , const int[], const float ,const int ,
                                                                   const int ,const int ));

    static QColor applyConvolution(const uchar *image,const int width, const int height,
                                                   const int kernelRadius, const int kernel[], const float kernelParameter,const int kernelWidth,
                                                   const int x,const int y);
private:
    QImage* currentImage;

};
#endif // IMAGEPROCESSING_H
