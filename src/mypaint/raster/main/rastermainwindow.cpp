/*
    Copyright © 2015 by The QTMyPaint Project

    This file is part of QTMyPaint, a Qt-based interface for MyPaint C++ library.

    QTMyPaint is libre software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QTMyPaint is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QTMyPaint. If not, see <http://www.gnu.org/licenses/>.
*/

#include "rastermainwindow.h"
#include "tapplicationproperties.h"

#include <QDebug>
#include <QFileDialog>
#include <QVBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

RasterMainWindow::RasterMainWindow(TupProject *project, const QString &winKey, TupProject::Mode context, int scene,
                                   const QColor contourColor, QWidget *parent): TMainWindow(winKey, parent)
{
    spaceContext = context;
    sceneIndex = scene;
    projectSize = project->getDimension();

    if (spaceContext == TupProject::RASTER_STATIC_BG_MODE) {
        // Raster Static Settings
        setWindowTitle(tr("Raster Static Background"));
        setWindowIcon(QIcon(THEME_DIR + "icons/raster_mode.png"));
    } else {
        // Raster Dynamic Settings
        setWindowTitle(tr("Raster Dynamic Background"));
        setWindowIcon(QIcon(THEME_DIR + "icons/dynamic_raster_mode.png"));
    }

    createTopResources();
    createCentralWidget(project, contourColor);

    colorWidget = new RasterColorWidget(contourColor, project->getBgColor(), this);
    connect(colorWidget, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
            this, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)));
    connect(colorWidget, SIGNAL(paintAreaEventTriggered(const TupPaintAreaEvent *)),
            this, SLOT(processColorEvent(const TupPaintAreaEvent *)));

    colorView = addToolView(colorWidget, Qt::LeftDockWidgetArea, Raster, "Brush Color", QKeySequence(tr("Shift+C")));

    brushesWidget = new RasterBrushesWidget(RASTER_RESOURCES_DIR + "brushes");
    connect(brushesWidget, SIGNAL(brushSelected(const QByteArray&)),
            rasterCanvas, SLOT(loadBrush(const QByteArray&)));

    brushesView = addToolView(brushesWidget, Qt::LeftDockWidgetArea, Raster, "Brushes", QKeySequence(tr("Shift+B")));
    brushesView->expandDock(true);

    status = new TupPaintAreaStatus(TupPaintAreaStatus::Raster);
    connect(status, SIGNAL(resetClicked()), this, SLOT(resetWorkSpaceTransformations()));
    connect(status, SIGNAL(safeAreaClicked()), this, SLOT(drawActionSafeArea()));
    connect(status, SIGNAL(gridClicked()), this, SLOT(drawGrid()));
    connect(status, SIGNAL(zoomChanged(qreal)), this, SLOT(setZoomFactor(qreal)));
    connect(status, SIGNAL(angleChanged(int)), this, SLOT(setRotationAngle(int)));
    connect(rasterCanvas, SIGNAL(rotated(int)), status, SLOT(updateRotationAngle(int)));
    setStatusBar(status);
    status->setZoomPercent("100");

    /*
    // Add a docked widget
    QDockWidget* dockBrush = new QDockWidget(tr("Brush Library"));
    brushesSelector = new RasterBrushSelector(RASTER_DIR + "brushes", nullptr);
    dockBrush->setWidget(brushesSelector);
    addDockWidget(Qt::LeftDockWidgetArea, dockBrush);

    connect(brushesSelector, SIGNAL(brushSelected(const QByteArray&)),
            rasterCanvas, SLOT(loadBrush(const QByteArray&)));
    */

    tabletIsActive = false;
}

RasterMainWindow::~RasterMainWindow()
{
    colorView = nullptr;
    delete colorView;

    brushesView = nullptr;
    delete brushesView;

    colorWidget = nullptr;
    delete colorWidget;

    brushesWidget = nullptr;
    delete brushesWidget;

    rasterCanvas = nullptr;
    delete rasterCanvas;

    status = nullptr;
    delete status;
}

void RasterMainWindow::createTopResources()
{
    QAction *exportAction = new QAction(tr("&Export as Image"), this);
    exportAction->setShortcuts(QKeySequence::Open);
    exportAction->setStatusTip(tr("Export as Image"));
    connect(exportAction, SIGNAL(triggered()), this, SLOT(exportImage()));

    QAction *closeAction = new QAction(tr("Exit Raster Mode"), this);
    closeAction->setShortcuts(QKeySequence::Quit);
    closeAction->setStatusTip(tr("Exit Raster Mode"));
    connect(closeAction, &QAction::triggered, this, &QWidget::close);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(exportAction);
    fileMenu->addAction(closeAction);
}

void RasterMainWindow::createCentralWidget(TupProject * project, const QColor contourColor)
{
    // Central widget:
    rasterCanvas = new RasterCanvas(project, contourColor, this);
    connect(rasterCanvas, SIGNAL(closeWindow()), this, SLOT(saveCanvas()));
    connect(rasterCanvas, SIGNAL(zoomIn()), this, SLOT(applyZoomIn()));
    connect(rasterCanvas, SIGNAL(zoomOut()), this, SLOT(applyZoomOut()));

    topBar = new QToolBar(tr("Raster actions"), this);
    topBar->setIconSize(QSize(16, 16));

    QPushButton *clearButton = new QPushButton(QIcon(THEME_DIR + "icons/new.png"), "", this);
    clearButton->setToolTip(tr("Clear Canvas"));
    clearButton->setShortcut(Qt::Key_Backspace);
    connect(clearButton, SIGNAL(clicked()), rasterCanvas, SLOT(clearCanvas()));

    topBar->addWidget(clearButton);

    QString imgPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/";
    if (spaceContext == TupProject::RASTER_STATIC_BG_MODE) {
        // Raster Static Settings
        imgPath += "static_bg.png";
    } else {
        // Raster Dynamic Settings
        imgPath += "dynamic_bg.png";
    }

    if (QFile::exists(imgPath)) {
        rasterCanvas->loadFromFile(imgPath);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "RasterMainWindow::createCentralWidget() - Image doesn't exist -> " + imgPath;
        #endif
    }

    QWidget *centralWidget = new QWidget();
    QVBoxLayout *centralLayout = new QVBoxLayout(centralWidget);
    centralLayout->addWidget(topBar);
    centralLayout->addWidget(rasterCanvas);

    setCentralWidget(centralWidget);
}

void RasterMainWindow::closeEvent(QCloseEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::closeEvent(QCloseEvent)";
    #endif

    saveCanvas();

    colorView->expandDock(false);
    brushesView->expandDock(false);

    TMainWindow::closeEvent(event);
}

void RasterMainWindow::resetWorkSpaceTransformations()
{
    rasterCanvas->resetWorkSpaceCenter(projectSize);
    status->setRotationAngle("0");
    status->setZoomPercent("100");
}

void RasterMainWindow::drawGrid()
{
    rasterCanvas->drawGrid(!rasterCanvas->getGridState());
}

void RasterMainWindow::drawActionSafeArea()
{
    rasterCanvas->drawActionSafeArea(!rasterCanvas->getSafeAreaState());
}

void RasterMainWindow::setZoomFactor(qreal factor)
{
    rasterCanvas->setZoom(factor);
}

void RasterMainWindow::applyZoomIn()
{
    qreal zoom = status->currentZoomFactor();
    if (zoom <= 495) {
        zoom += 5;
        status->setZoomPercent(QString::number(zoom));
    }
}

void RasterMainWindow::applyZoomOut()
{
    qreal zoom = status->currentZoomFactor();
    if (zoom >= 15) {
        zoom -= 5;
        status->setZoomPercent(QString::number(zoom));
    }
}

void RasterMainWindow::setRotationAngle(int angle)
{
    rasterCanvas->setRotationAngle(angle);
}

void RasterMainWindow::processColorEvent(const TupPaintAreaEvent *event)
{
    QColor color = qvariant_cast<QColor>(event->getData());
    if (event->getAction() == TupPaintAreaEvent::ChangePenColor) {
        rasterCanvas->updateBrushColor(color);
    } else if (event->getAction() == TupPaintAreaEvent::ChangeBgColor) {
        rasterCanvas->setBgColor(color);
    }
}

void RasterMainWindow::setTabletDevice(QTabletEvent* event)
{
    rasterCanvas->setTabletDevice(event);
}

void RasterMainWindow::openProject()
{
    // Path
    QString initPath = QDir::homePath();
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open Image"), initPath);

    if (filePath.isEmpty())
        return; // false;

    rasterCanvas->loadFromFile(filePath);
}

void RasterMainWindow::exportImage()
{
    // Path
    QString initPath = QDir::homePath() + "/static_bg.png";
    QString filePath = QFileDialog::getSaveFileName(this, tr("Export Image"), initPath);
    if (filePath.isEmpty())
        return; // false;

    rasterCanvas->saveToFile(filePath);
}

void RasterMainWindow::saveCanvas()
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::saveCanvas()";
    #endif

    // SQA: Only render if there is at least one stroke
    QString imgPath = RASTER_BG_DIR + QString::number(sceneIndex) + "/bg/";

    if (QDir().mkpath(imgPath)) {
        QString prefix = "dynamic_bg.png";
        if (spaceContext == TupProject::VECTOR_STATIC_BG_MODE)
            prefix = "static_bg.png";

        rasterCanvas->saveToFile(imgPath + prefix);
        emit closeWindow(imgPath + prefix);
    } else {
        #ifdef TUP_DEBUG
            qDebug() << "RasterMainWindow::saveCanvas() - Error while creating raster background path!";
            qDebug() << "Image Path: " << imgPath;
        #endif
    }
}

void RasterMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    rasterCanvas->setSize(this->centralWidget()->rect().size());
}

void RasterMainWindow::keyPressEvent(QKeyEvent *event)
{
    #ifdef TUP_DEBUG
        qDebug() << "RasterMainWindow::keyPressEvent()";
    #endif

    if (event->key() == Qt::Key_F11 || event->key() == Qt::Key_Escape)
        saveCanvas();
}
