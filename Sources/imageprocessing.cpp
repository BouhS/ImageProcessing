#include "Headers/imageprocessing.h"
#include "ui_imageprocessing.h"

ImageProcessing::ImageProcessing(QImage* image)
{
    currentImage = image;
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
Convert image to greyScale
*/
QImage* ImageProcessing::convertToGrayScale(const  uchar* imageData,const int width,const int height,const QImage::Format format)
{
    QImage* grayScaleImage = new QImage(width,height,format);
    uchar* grayScaleImageData = grayScaleImage->bits();
    for(int i= 0;i<height * width * 4; i+=4 )
    {
        float greyScaleValue = 0.299f * imageData[i] + 0.587f * imageData[i+1] + 0.114f *imageData[i+2];
        //Red
        grayScaleImageData[i] = greyScaleValue;
        //Green
        grayScaleImageData[i+1] = greyScaleValue;
        //Blue
        grayScaleImageData[i+2] = greyScaleValue;
        //Alpha
        grayScaleImageData[i+3] = imageData[i +3];
    }
    return grayScaleImage;
}

QImage* ImageProcessing::meanBlur(const  uchar* imageData,const int width, const int height,const QImage::Format format)
{
    const int kernel[9] ={1,1,1,
                          1,1,1,
                          1,1,1};

    return applyFilter(imageData, width, height, format, 1, kernel, 9.0f , &ImageProcessing::applyConvolution);
}

QImage* ImageProcessing::gaussianBlur3x3(const  uchar* imageData,const int width, const int height,const QImage::Format format)
{
    const int kernel[9] ={1,2,1,
                          2,4,2,
                          1,2,1};


    return applyFilter(imageData, width, height, format, 1, kernel, 16.0f , &ImageProcessing::applyConvolution);
}

QImage* ImageProcessing::gaussianBlur5x5(const  uchar* imageData,const int width, const int height,const QImage::Format format)
{
    const int kernel[25] ={1,4,6,4,1,
                          4,16,24,16,4,
                          6,24,36,24,6,
                          4,16,24,16,4,
                           1,4,6,4,1};

    return applyFilter(imageData, width, height, format, 2, kernel, 246.0f , &ImageProcessing::applyConvolution);
}

QImage* ImageProcessing::medianFilter(const uchar* imageData, const int width, const int height, QImage::Format format)
{
    QImage* filteredImage = new QImage(width,height, format);
    uchar* filteredImageData = filteredImage->bits();
    int kernelRadius = 1;

    //list of neighborhood values
    std::vector<int> medianList;
    medianList.reserve(9);
    for(int i=0; i<width; i++)
    {
        for(int j=0; j<height; j++)
        {
            medianList.clear();

            for(int ki=-kernelRadius; ki<=kernelRadius; ki++)
            {
                int x = fmax(fmin(i+ki,width-1),0);
                for(int kj=-kernelRadius; kj<=kernelRadius; kj++)
                {
                    int y = fmax(fmin(j+kj,height-1),0);
                    int index = 4*x+ y*width*4;
                    medianList.push_back(imageData[index]);
                }
            }
            // Find the median value
            std::sort(medianList.begin(),medianList.end());
            int medianValue = medianList[medianList.size()/2 ];
            int id = 4*i+ j*width*4;
            filteredImageData[id] = medianValue;
            filteredImageData[id+1] = medianValue;
            filteredImageData[id+2] = medianValue;
            filteredImageData[id+3] = 255.0f;
        }
    }
    return filteredImage;
}

QImage* ImageProcessing::variationFilter(const uchar* imageData, const int width, const int height, QImage::Format format)
{

    QImage* filteredImage = new QImage(width, height, format);
    uchar* filteredImageData = filteredImage->bits();


    int kernelRadius = 2;
    int kernelSize = (kernelRadius*2+1)*(kernelRadius*2+1);

    //list of neighborhood values
    std::vector<float> neighborhoodValuesList;
    neighborhoodValuesList.reserve(kernelSize);

    for(int x=0; x<width; x++)
    {
        for(int y=0; y<height; y++)
        {
            int id = 4*x+y*width*4;
            QColor pixelColor = QColor(imageData[id],
                                         imageData[id+1],
                                         imageData[id+2]);

            float finalWeight=0.0f;
            //finalColor is a float because the image is supposed to be grey (r = g = b)
            float finalColor =0;
            neighborhoodValuesList.clear();

            for(int i=-kernelRadius; i<=kernelRadius; i++)
            {
                int xNeighbor = fmax(fmin(x+i, width-1), 0);

                for(int j=-kernelRadius; j<=kernelRadius; j++)
                {
                    int yNeighbor = fmax(fmin(y+j, height-1), 0);

                    float weight = 5.0f;

                    int index = 4*xNeighbor+ yNeighbor*width*4;
                    QColor neighborColor = QColor(imageData[index],
                                                 imageData[index+1],
                                                 imageData[index+2]);

                    if(pixelColor.red() != neighborColor.red())
                    {
                        weight = abs(pixelColor.red() - neighborColor.red());
                    }
                    neighborhoodValuesList.push_back(1.0/weight * neighborColor.red());
                    finalWeight += 1.0/weight;
                }
            }

            for(int k=0; k<(int)neighborhoodValuesList.size(); k++)
            {
                finalColor = finalColor + neighborhoodValuesList[k] /(finalWeight);
            }

            filteredImageData[id] = finalColor;
            filteredImageData[id+1] = finalColor;
            filteredImageData[id+2] = finalColor;
            filteredImageData[id+3] = 255.0f;
        }
    }

    return filteredImage;
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

QImage* ImageProcessing::gradientFilter(const  uchar* imageData,const int width, const int height,const QImage::Format format)
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

    QImage* imageFiltered = new QImage(width, height,format);
    uchar* imageFilteredData = imageFiltered->bits();
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

                    int index = 4*x + y*width *4;
                    int red = imageData[index];
                    int green = imageData[index +1] ;
                    int blue = imageData[index +2] ;

                    float h = kernelX[x_k + y_k*kernelWidth];
                    float hY = kernelY[x_k + y_k*kernelWidth];

                    gradientX[0]+= (red * h);
                    gradientX[1]+=(green * h);
                    gradientX[2]+=(blue * h);

                    gradientY[0]+= (red * hY);
                    gradientY[1]+=(green * hY);
                    gradientY[2]+=(blue * hY);

                }
            }
            gradientX[0] /= (c+2);
            gradientX[1] /= (c+2);
            gradientX[2] /= (c+2);

            gradientY[0] /= (c+2);
            gradientY[1] /= (c+2);
            gradientY[2] /= (c+2);

            int id = 4*i + j*width *4;
            //red
            imageFilteredData[id] = sqrt(pow(gradientX[0],2)+pow(gradientY[0],2));
            //green
            imageFilteredData[id+1] = sqrt(pow(gradientX[1],2)+pow(gradientY[1],2));
            //blue
            imageFilteredData[id+2] =sqrt(pow(gradientX[2],2)+pow(gradientY[2],2));
            //alpha
            imageFilteredData[id+3] = 255.0f;

        }
    }
    return imageFiltered;

}


QImage* ImageProcessing::horizontalSobelGradientFilter(const  uchar* imageData,const int width, const int height,const QImage::Format format)
{
    const int c = 2;

    const int kernel[9] ={-1,0,1,
                          -c,0,c,
                          -1,0,1};


    return applyFilter(imageData, width, height, format, 1, kernel, c+2 , &ImageProcessing::applyConvolution);
}

QImage* ImageProcessing::verticalSobelGradientFilter(const  uchar* imageData,const int width, const int height,const QImage::Format format)
{
    const int c = 2;

    const int kernel[9] ={-1,-c,-1,
                           0,0,0,
                           1,c,1};

    return applyFilter(imageData, width, height, format, 1, kernel, c+2 , &ImageProcessing::applyConvolution);
}

QImage* ImageProcessing::applyFilter(const uchar *imageData, const int width, const int height, const QImage::Format format, const int kernelRadius, const int kernel[], const float kernelParameter,
                                          QColor (*convolution)(const uchar *,const int, const int,
                                                               const int , const int[], const float ,const int ,
                                                               const int ,const int ))
{
    const int kernelWidth = 2*kernelRadius +1;
    QImage* imageFiltered = new QImage(width, height, format);
    uchar* imageFilteredData = imageFiltered->bits();

    for(int x= 0 ; x<width; x++)
    {
        for(int y= 0 ; y<height; y++)
        {
            QColor color = convolution(imageData,width,height,kernelRadius,kernel,kernelParameter,kernelWidth,x,y);
            int index = 4*x + y * width*4 ;
            imageFilteredData[index] = color.red();
            imageFilteredData[index +1] = color.green();
            imageFilteredData[index +2] = color.blue();
            imageFilteredData[index +3] = color.alpha();
        }
    }
    return imageFiltered;
}

// A simple convolution function
QColor ImageProcessing::applyConvolution(const uchar *imageData,const int width, const int height,
                                               const int kernelRadius, const int kernel[], const float kernelParameter,const int kernelWidth,
                                               const int x,const int y)
{
    int r = 0;
    int g = 0;
    int b = 0;

    for(int kx=-kernelRadius; kx<=kernelRadius; kx++)
    {
        int iKernel = kx + kernelRadius;
        //index i of the neighboor pixel
        int i = fmax(fmin(x+kx,width-1),0);
        for(int ky=-kernelRadius; ky<=kernelRadius; ky++)
        {
            int jKernel = ky + kernelRadius;
            //index j of the neighboor pixel
            int j = fmax(fmin(y+ky,height-1),0);
            QColor imageColor =  QColor(imageData[4*i +j* width*4], imageData[4*i +j* width*4 +1],imageData[4*i +j* width*4+2] ,imageData[4*i +j* width*4+3]) ;

            float h = kernel[iKernel+ jKernel*kernelWidth] / kernelParameter;
            r = fminf( r + imageColor.red()   * h, 255.0f);
            g = fminf( g + imageColor.green()   * h, 255.0f);
            b = fminf( b + imageColor.blue()   * h, 255.0f);

        }
    }
    return QColor(abs(r),abs(g),abs(b));
}
