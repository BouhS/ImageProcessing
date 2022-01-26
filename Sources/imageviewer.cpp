/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#ifndef QT_NO_PRINTER
#include <QPrintDialog>
#endif

#include "Headers/imageviewer.h"

ImageViewer::ImageViewer()
   : imageLabel(new QLabel)
   , scrollArea(new QScrollArea)
   , scaleFactor(1)
{
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(false);
    setCentralWidget(scrollArea);

    createActions();

    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
}


bool ImageViewer::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    setImage(newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);
    return true;
}

void ImageViewer::setImage(const QImage &newImage)
{
    image = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();
}

bool ImageViewer::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(image)) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/png");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("png");
}

void ImageViewer::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::saveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}
}

void ImageViewer::print()
{
  //  Q_ASSERT(imageLabel->pixmap());
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap().size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap().rect());
        painter.drawPixmap(0, 0, imageLabel->pixmap());
    }
#endif
}

void ImageViewer::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData()) {
        if (mimeData->hasImage()) {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void ImageViewer::paste()
{
#ifndef QT_NO_CLIPBOARD
    const QImage newImage = clipboardImage();
    if (newImage.isNull()) {
        statusBar()->showMessage(tr("No image in clipboard"));
    } else {
        setImage(newImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(newImage.width()).arg(newImage.height()).arg(newImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void ImageViewer::zoomIn()
{
    scaleImage(1.25);
}

void ImageViewer::zoomOut()
{
    scaleImage(0.8);
}

void ImageViewer::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void ImageViewer::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}

void ImageViewer::meanBlur()
{
   QImage* result =  imageProcessor->meanBlur(&image);
   if(result != nullptr)
   {
       setImage(*result);
       QMessageBox::warning(this, tr("Warning"),tr("Blur applied"));
   }
   else
   {
       QMessageBox::warning(this, tr("Warning"),tr("No image found"));
   }
}

void ImageViewer::gaussianBlur3x3()
{
   QImage* result =  imageProcessor->gaussianBlur3x3(&image);
   if(result != nullptr)
   {
       setImage(*result);
       QMessageBox::warning(this, tr("Warning"),tr("Blur applied"));
   }
   else
   {
       QMessageBox::warning(this, tr("Warning"),tr("No image found"));
   }
}

void ImageViewer::gaussianBlur5x5()
{
   QImage* result =  imageProcessor->gaussianBlur5x5(&image);
   if(result != nullptr)
   {
       setImage(*result);
       QMessageBox::warning(this, tr("Warning"),tr("Blur applied"));
   }
   else
   {
       QMessageBox::warning(this, tr("Warning"),tr("No image found"));
   }
}

void ImageViewer::medianFilter()
{
   QImage* result =  imageProcessor->medianFilter(&image);
   if(result != nullptr)
   {
       setImage(*result);
       QMessageBox::warning(this, tr("Warning"),tr("Blur applied"));
   }
   else
   {
       QMessageBox::warning(this, tr("Warning"),tr("No image found"));
   }
}

void ImageViewer::variationFilter()
{
   QImage* result =  imageProcessor->variationFilter(&image);
   if(result != nullptr)
   {
       setImage(*result);
       QMessageBox::warning(this, tr("Warning"),tr("Blur applied"));
   }
   else
   {
       QMessageBox::warning(this, tr("Warning"),tr("No image found"));
   }
}

void ImageViewer::showHistogram()
{
//drawBarChart();
    if(image.isNull())
    {
        QMessageBox::warning(this, tr("Warning"),tr("No image !"));
    }
    else
    {
        std::vector<int> redHistogram;
        redHistogram.reserve(256);
        std::vector<int> greenHistogram;
        greenHistogram.reserve(256);
        std::vector<int> blueHistogram;
        blueHistogram.reserve(256);
        std::vector<QBarSet*> barSets;
        barSets.reserve(256);

        QBarSeries *series = new QBarSeries();

        for(int i=0; i < 256; i++)
        {
            redHistogram.push_back(0);
            greenHistogram.push_back(0);
            blueHistogram.push_back(0);
        }

       imageProcessor->computeHistogram(&image,&redHistogram,&greenHistogram,&blueHistogram);


       int size = redHistogram.size();
       float imageSize = image.width()*image.height();
       float total = 0;
       float maxValue = 0.f;
       for(int i=0; i<size;i++)
       {
           QBarSet *set = new QBarSet(""+i);
           set->setBrush(Qt::black);
           set->setBorderColor(QColor(0,0,0));
           set->setColor(QColor(0,0,0));
           float value = redHistogram[i]/imageSize;
           set->append(value);
           if( maxValue < value)
               maxValue = value;
           total += value;
           series->append(set);
       }

      // qDebug() <<"total " <<total;

       QChart *chart = new QChart();
       chart->addSeries(series);
       chart->setTitle("Histogram");

       QStringList categories;
       categories << "";

       QValueAxis *axisY = new QValueAxis();
       axisY->setRange(0,maxValue);
       chart->addAxis(axisY, Qt::AlignLeft);
       series->attachAxis(axisY);

       chart->legend()->setVisible(false);
       chart->legend()->setAlignment(Qt::AlignBottom);

       QChartView *chartView = new QChartView(chart);

      // chartView->setRenderHint(QPainter::Antialiasing);
       chartView->resize(800,500);
       chartView->show();
    }
 //  QMessageBox::warning(this, tr("Warning"),tr("Histogram applied"));

}

void ImageViewer::drawBarChart()
{

        //List of different BarSets . Each barset contains the number of pixels of that index value
        vector<QBarSet *> barSetList(256);

        QBarSeries *series = new QBarSeries();


        for(int i=0; i<256;i++)
        {
            QBarSet *set = new QBarSet(""+i);
            set->setColor(QColor(0,0,0));
            set->append(i);
            barSetList.push_back(set);
            series->append(set);
        }



    //![3]
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Simple barchart example");
        chart->setAnimationOptions(QChart::SeriesAnimations);
    //![3]

    //![4]
        QStringList categories;
        categories << "Red" << "Green" << "Blue" ;
        QBarCategoryAxis *axisX = new QBarCategoryAxis();
        axisX->append(categories);
        chart->addAxis(axisX, Qt::AlignBottom);
        series->attachAxis(axisX);

        QValueAxis *axisY = new QValueAxis();
        axisY->setRange(0,15);
        chart->addAxis(axisY, Qt::AlignLeft);
        series->attachAxis(axisY);
    //![4]

    //![5]
        chart->legend()->setVisible(false);
        chart->legend()->setAlignment(Qt::AlignBottom);
    //![5]

    //![6]
        QChartView *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);
    //![6]
    //!
    chartView->show();
}

void ImageViewer::gradientFilter()
{
 QImage* result = imageProcessor->gradientFilter(&image);
 if(result != nullptr)
 {
     setImage(*result);
     QMessageBox::warning(this, tr("Warning"),tr("Filter applied"));
 }
 else
 {
     QMessageBox::warning(this, tr("Warning"),tr("No image found"));
 }

}


void ImageViewer::horizontalGradientFilter()
{
    QImage* result = imageProcessor->horizontalSobelGradientFilter(&image);
    if(result != nullptr)
    {
        setImage(*result);
        QMessageBox::warning(this, tr("Warning"),tr("Filter applied"));
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"),tr("No image found"));
    }

}

void ImageViewer::verticalGradientFilter()
{
    QImage* result = imageProcessor->verticalSobelGradientFilter(&image);
    if(result != nullptr)
    {
        setImage(*result);
        QMessageBox::warning(this, tr("Warning"),tr("Filter applied"));
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"),tr("No image found"));
    }

}

void ImageViewer::about()
{
    QMessageBox::about(this, tr("About Image Viewer"),
            tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
               "and QScrollArea to display an image. QLabel is typically used "
               "for displaying a text, but it can also display an image. "
               "QScrollArea provides a scrolling view around another widget. "
               "If the child widget exceeds the size of the frame, QScrollArea "
               "automatically provides scroll bars. </p><p>The example "
               "demonstrates how QLabel's ability to scale its contents "
               "(QLabel::scaledContents), and QScrollArea's ability to "
               "automatically resize its contents "
               "(QScrollArea::widgetResizable), can be used to implement "
               "zooming and scaling features. </p><p>In addition the example "
               "shows how to use QPainter to print an image.</p>"));
}

void ImageViewer::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &ImageViewer::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &ImageViewer::saveAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &ImageViewer::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("&Exit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &ImageViewer::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), this, &ImageViewer::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &ImageViewer::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &ImageViewer::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &ImageViewer::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &ImageViewer::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *filtersMenu = menuBar()->addMenu(tr("&Filters"));
    filtersMenu->addAction(tr("&MeanBlur"), this, &ImageViewer::meanBlur);
    filtersMenu->addAction(tr("&GaussianBlur"), this, &ImageViewer::gaussianBlur3x3);
    filtersMenu->addAction(tr("&GaussianBlur5x5"), this, &ImageViewer::gaussianBlur5x5);
    filtersMenu->addAction(tr("&MedianFilter"), this, &ImageViewer::medianFilter);
    filtersMenu->addAction(tr("&VariationFilter"), this, &ImageViewer::variationFilter);

    QMenu *imageMenu = menuBar()->addMenu(tr("&Image"));
    imageMenu->addAction(tr("&Histogram"), this, &ImageViewer::showHistogram);

    QMenu *edgeDetectionMenu = menuBar()->addMenu(tr("&Edge Detection"));
    edgeDetectionMenu->addAction(tr("&Gradient"), this, &ImageViewer::gradientFilter);
    edgeDetectionMenu->addAction(tr("&HorizontalGradient"), this, &ImageViewer::horizontalGradientFilter);
    edgeDetectionMenu->addAction(tr("&VerticalGradient"), this, &ImageViewer::verticalGradientFilter);

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &ImageViewer::about);
    helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void ImageViewer::updateActions()
{
    saveAsAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void ImageViewer::scaleImage(double factor)
{
   // Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap().size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
