#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H
#include "imageprocessing.h"

#include <QMainWindow>

#include <QScrollBar>
#include <QLabel>
#include <QScrollArea>
#include <QPrinter>
#include <QBarSet>
#include <QBarSeries>
#include <QChart>
#include <QChartView>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include <QColor>

#include <thread>
#include <functional>
#include <string>
using namespace std;

QT_BEGIN_NAMESPACE
namespace Ui { class ImageViewer; }
QT_END_NAMESPACE

class ImageViewer : public QMainWindow
{
    Q_OBJECT

public:
    ImageViewer();
    bool loadFile(const QString &);

private slots:
    void open();
    void saveAs();
    void print();
    void copy();
    void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    //
    void grayscale();
    // Blur
    void meanBlur();
    void gaussianBlur3x3();
    void gaussianBlur5x5();
    void medianFilter();
    void variationFilter();
    // Histogram
    void showHistogram();
    // Edge detection
    void gradientFilter();
    void horizontalGradientFilter();
    void verticalGradientFilter();
    void about();
    //

private:
    void createActions();
    void createMenus();
    void updateActions();
    bool saveFile(const QString &fileName);
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    QImage image;
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor;
    ImageProcessing *imageProcessor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QAction *saveAsAct;
    QAction *printAct;
    QAction *copyAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QMenu *filtersMenu;
    QMenu *imageMenu;
    QMenu *edgeDetectionMenu;
};
#endif // IMAGEVIEWER_H
