#include "Headers/imageprocessing.h"
#include "ui_imageprocessing.h"

ImageProcessing::ImageProcessing(QImage* image)
{
}

ImageProcessing::~ImageProcessing()
{
}

void ImageProcessing::createNewImage()
{
    currentImage = new QImage();
}

void ImageProcessing::setAsCurrentImage(QImage *image)
{
    currentImage = image;
}

/*
Blur an ima
*/
QImage* ImageProcessing::blur(const QImage* image, const int kernel[], const float kernelParameter, const int kernelRadius)
{
    if(image !=nullptr)
    {

         QImage* filteredImage = new QImage(*image);
         qDebug() <<"----------- height "<<image->height() << "width"<< image->width() ;


         QColor color = QColor(0,0,0);
         const int width = image->width();
         const int height = image->height() ;
         const int kWidth = 2*kernelRadius+1;
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            {
                color = QColor(0,0,0);
                for(int ki=-kernelRadius; ki<=kernelRadius; ki++)
                {
                    int x_k = ki+ kernelRadius;
                    int x = fmax(fmin(i+ki,width-1),0);
                    for(int kj=-kernelRadius; kj<=kernelRadius; kj++)
                    {

                        int y_k = kj + kernelRadius;
                        int y = fmax(fmin(j+kj,height-1),0);
                        QColor imageColor =  filteredImage->pixelColor(x,y);
                        float h = kernel[x_k+ y_k*kWidth] /kernelParameter;

                        color.setRed( fminf(color.red()     + imageColor.red()   * h, 255.0f));
                        color.setGreen( fminf(color.green() + imageColor.green() * h, 255.0f));
                        color.setBlue( fminf(color.blue()   + imageColor.blue()  * h, 255.0f));
                    }
                }
                filteredImage->setPixelColor(i, j,color);
            }

        }

        return filteredImage;

    }
    else
    {
        return nullptr;
    }

}

QImage* ImageProcessing::meanBlur(const QImage* image)
{
    const int kernel[9] ={1,1,1,
                          1,1,1,
                          1,1,1};

    return blur(image,kernel,9.0f,1);


}

QImage* ImageProcessing::gaussianBlur3x3(const QImage* image)
{
    const int kernel[9] ={1,2,1,
                          2,4,2,
                          1,2,1};

    return blur(image,kernel,16.0f,1);
}

QImage* ImageProcessing::gaussianBlur5x5(const QImage* image)
{
    const int kernel[25] ={1,4,6,4,1,
                          4,16,24,16,4,
                          6,24,36,24,6,
                          4,16,24,16,4,
                           1,4,6,4,1};

    return blur(image,kernel,246.0f,2);
}

QImage* ImageProcessing::medianFilter(const QImage* image)
{
    if(image !=nullptr)
    {

         QImage* filteredImage = new QImage(*image);
         int kernelRadius = 1;

         QColor color = QColor(0,0,0);
         const int width = image->width();
         const int height = image->height() ;

         //list of neighborhood values
         std::vector<int> medianList;
         medianList.reserve(9);
        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            {
                medianList.clear();
                color = QColor(0,0,0);
                for(int ki=-kernelRadius; ki<=kernelRadius; ki++)
                {
                    int x = fmax(fmin(i+ki,width-1),0);
                    for(int kj=-kernelRadius; kj<=kernelRadius; kj++)
                    {
                        int y = fmax(fmin(j+kj,height-1),0);
                        QColor imageColor =  filteredImage->pixelColor(x,y);

                        medianList.push_back(imageColor.red());

                    }
                }
                std::sort(medianList.begin(),medianList.end());
                int medianValue = medianList[medianList.size()/2 ];
                color.setRed(medianValue);
                color.setGreen(medianValue);
                color.setBlue(medianValue);
                filteredImage->setPixelColor(i, j,color);
            }

        }

        return filteredImage;

    }
    else
    {
        return nullptr;
    }

}

QImage* ImageProcessing::variationFilter(const QImage* image)
{
    if(image !=nullptr)
    {

         QImage* filteredImage = new QImage(*image);
         int kernelRadius = 2;

         QColor color = QColor(0,0,0);
         const int width = image->width();
         const int height = image->height() ;

         //list of neighborhood values
         std::vector<float> kernel;
         int kernelSize = (kernelRadius*2+1)*(kernelRadius*2+1);
         kernel.reserve(kernelSize);

        for(int i=0; i<width; i++)
        {
            for(int j=0; j<height; j++)
            {
                QColor imageColorij = image->pixelColor(i,j);
                color = QColor(0,0,0);
                float totalH=0.0f;
                float colorF=0;
                kernel.clear();
                for(int ki=-kernelRadius; ki<=kernelRadius; ki++)
                {
                    int x = fmax(fmin(i+ki,width-1),0);
                    for(int kj=-kernelRadius; kj<=kernelRadius; kj++)
                    {
                        int y = fmax(fmin(j+kj,height-1),0);

                        float h = 5.0f;
                        QColor imageColorkl =  filteredImage->pixelColor(x,y);
                        if(imageColorij.red() != imageColorkl.red())
                        {
                            h = abs(imageColorij.red() - imageColorkl.red());

                        }
                        kernel.push_back(1.0/h * imageColorkl.red());
                        totalH += 1.0/h;
                    }
                }
                for(int k=0; k<kernel.size(); k++)
                {
                    colorF = colorF + kernel[k] /(totalH);
                }
                color.setRed(colorF );
                color.setGreen(colorF );
                color.setBlue(colorF);
                filteredImage->setPixelColor(i, j,color);
            }

        }

        return filteredImage;

    }
    else
    {
        return nullptr;
    }

}

void ImageProcessing::computeHistogram(const QImage* image,std::vector<int> *redHistogram,std::vector<int> *greenHistogram,std::vector<int> *blueHistogram)
{
    const int width = image->width();
    const int height = image->height();

    const int nbThreads = 4;
    vector<thread> threads;
    vector< vector<int> *> *histogramVector = new vector< vector<int> *>(nbThreads);
    // Size of the section handled by one thread
    int  size = width/nbThreads +1;
    int histoSize = redHistogram->size();
    for(int i=0; i< nbThreads; i++)
    {

        // Initialize vector that will be accessed by threads
        histogramVector->at(i) = (new vector<int>(histoSize));
        for(int j=0; j< histoSize; j++)
        {
            ((*histogramVector)[i])->at(j) = 0;
        }

        // Section the thread will handle
        int x_start =  min(width,i*size);
        int x_end = min(width,x_start +  size);

        //Create thread
        threads.push_back(thread(fillHistogram,*image,x_start,x_end,height,std::ref(histogramVector),i));

    }

    // Join all threads
    for_each(threads.begin(),threads.end(),
        mem_fn(&thread::join));

    // Add all values computed by the threads to get the final value
    for(int i=0; i< nbThreads; i++)
    {
        for(int j=0; j< histoSize; j++)
        {
            (*redHistogram)[j] += histogramVector->at(i)->at(j);
        }

    }


}

void ImageProcessing::fillHistogram(const QImage image,const int x_start,const int x_end, const int height, std::vector< std::vector<int> *> * histograms, const int i)
{
    for(int x=x_start; x<x_end; x++)
        {
        for(int y=0; y<height; y++)
            {
                QColor color = image.pixelColor(x,y);
                histograms->at(i)->at(color.red()) += 1;
            }
    }
}

QImage* ImageProcessing::gradientFilter(const QImage *image)
{
    // c = 2 : Sobel ; c = 1 : Prewitt
    const int c = 2;
    const int kernelX[9] ={-1,0,1,
                          -c,0,c,
                          -1,0,1};

    const int kernelY[9] ={-1,-c,-1,
                           0,0,0,
                           1,c,1};
    const int kernelRadius = 1;
    const int kernelWidth = 3;

    const int width = image->width();
    const int height = image->height();
    QImage* imageFiltered = new QImage(image->width(), image->height(),image->format());
    for(int i= 0 ; i<width; i++)
    {
        for(int j= 0 ; j<height; j++)
        {
            float gradientX[3] = {0.0f,0.0f,0.0f};
            float gradientY[3] = {0.0f,0.0f,0.0f};
            for(int ki=-kernelRadius; ki<=kernelRadius; ki++)
            {
                int x_k = ki+ kernelRadius;
                int x = fmax(fmin(i+ki,width-1),0);
                for(int kj=-kernelRadius; kj<=kernelRadius; kj++)
                {
                    int y_k = kj+ kernelRadius;
                    int y = fmax(fmin(j+kj,height-1),0);
                    QColor imageColor =  image->pixelColor(x,y);

                    float h = kernelX[x_k + y_k*kernelWidth];
                    float hY = kernelY[x_k + y_k*kernelWidth];

                    gradientX[0]+= (imageColor.red() * h);
                    gradientX[1]+=(imageColor.green() * h);
                    gradientX[2]+=(imageColor.blue() * h);

                    gradientY[0]+= (imageColor.red() * hY);
                    gradientY[1]+=(imageColor.green() * hY);
                    gradientY[2]+=(imageColor.blue() * hY);

                }
            }
            gradientX[0] /= (c+2);
            gradientX[1] /= (c+2);
            gradientX[2] /= (c+2);

            gradientY[0] /= (c+2);
            gradientY[1] /= (c+2);
            gradientY[2] /= (c+2);

            imageFiltered->setPixelColor(i,j,qRgb(sqrt(pow(gradientX[0],2)+pow(gradientY[0],2)),
                                                  sqrt(pow(gradientX[0],2)+pow(gradientY[0],2)),
                                                  sqrt(pow(gradientX[2],2)+pow(gradientY[2],2))));
        }
    }
    return imageFiltered;

}

QImage* ImageProcessing::horizontalSobelGradientFilter(const QImage *image)
{

    QImage* imageFiltered = new QImage(image->width(), image->height(),image->format());
    for(int i= 0 ; i<image->width(); i++)
    {
        for(int j= 0 ; j<image->height(); j++)
        {
            if((i-1) >=0)
            {
               QColor color = QColor(abs(image->pixelColor(i,j).red() - image->pixelColor(i-1,j).red()),
                                        abs(image->pixelColor(i,j).green() - image->pixelColor(i-1,j).green()),
                                        abs(image->pixelColor(i,j).blue() - image->pixelColor(i-1,j).blue()));
                imageFiltered->setPixelColor(i,j,color);
            }
        }
    }
    return imageFiltered;
}

QImage* ImageProcessing::verticalSobelGradientFilter(const QImage *image)
{

    QImage* imageFiltered = new QImage(image->width(), image->height(),image->format());
    for(int i= 0 ; i<image->width(); i++)
    {
        for(int j= 0 ; j<image->height(); j++)
        {
            if( (j-1) >=0 )
            {
               QColor color = QColor(abs(image->pixelColor(i,j).red() - image->pixelColor(i,j-1).red()),
                                        abs(image->pixelColor(i,j).green() - image->pixelColor(i,j-1).green()),
                                        abs(image->pixelColor(i,j).blue() - image->pixelColor(i,j-1).blue()));
                imageFiltered->setPixelColor(i,j,color);
            }
        }
    }
    return imageFiltered;
}
