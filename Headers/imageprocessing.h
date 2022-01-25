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
    QImage* blur(const QImage* image, const int kernel[], const float kernelParameter, const int kernelRadius);
    QImage* meanBlur(const QImage* image);
    QImage* gaussianBlur3x3(const QImage* image);
    QImage* gaussianBlur5x5(const QImage* image);
    QImage* medianFilter(const QImage* image);
    QImage* variationFilter(const QImage* image);
    void computeHistogram(const QImage* image,std::vector<int> *redHistogram,std::vector<int> *greenHistogram,std::vector<int> *blueHistogram);
    static void fillHistogram(const QImage image,const int x_start,const int x_end, const int height, vector< vector<int> *> * histograms, const int i);

   // static void fillHistogram(const QImage image,const int x_start,const int x_end, const int height, std::vector<int> *redHistogram,std::vector<int> *greenHistogram,std::vector<int> *blueHistogram, const int i);

private:
    QImage* currentImage;

};
#endif // IMAGEPROCESSING_H
