/***************************************************************************
 *   Project TUPITUBE DESK                                                *
 *   Project Contact: info@maefloresta.com                                 *
 *   Project Website: http://www.maefloresta.com                           *
 *   Project Leader: Gustav Gonzalez <info@maefloresta.com>                *
 *                                                                         *
 *   Developers:                                                           *
 *   2010:                                                                 *
 *    Gustavo Gonzalez / xtingray                                          *
 *                                                                         *
 *   KTooN's versions:                                                     * 
 *                                                                         *
 *   2006:                                                                 *
 *    David Cuadrado                                                       *
 *    Jorge Cuadrado                                                       *
 *   2003:                                                                 *
 *    Fernado Roldan                                                       *
 *    Simena Dinas                                                         *
 *                                                                         *
 *   Copyright (C) 2010 Gustav Gonzalez - http://www.maefloresta.com       *
 *   License:                                                              *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "tupfilemanager.h"
#include "tconfig.h"
#include "tupproject.h"
#include "tupscene.h"
#include "tuplayer.h"
#include "tuplibrary.h"
#include "tuppackagehandler.h"
#include "talgorithm.h"
#include "tbackupdialog.h"

#include <QDir>
#include <QScreen>
#include <QMessageBox>

TupFileManager::TupFileManager() : QObject()
{
}

TupFileManager::~TupFileManager()
{
}

bool TupFileManager::save(const QString &fileName, TupProject *project)
{
    #ifdef TUP_DEBUG
        qWarning() << "[TupFileManager::save()] - Saving file -> " << fileName;
    #endif

    QFileInfo info(fileName);
    QString name = info.baseName();	
    QString oldDirName = CACHE_DIR + project->getName();
    QDir projectDir(oldDirName);
    bool ok;

    // Project name has been changed by the user
    if (name.compare(project->getName()) != 0) {
        #ifdef TUP_DEBUG
            qDebug() << "[TupFileManager::save()] - User changed project's name...";
        #endif
        project->setProjectName(name);
        projectDir.setPath(CACHE_DIR + name);    
        project->getLibrary()->updatePaths(CACHE_DIR + name);
        if (!projectDir.exists()) {
            // Update the cache path with new project's name
            if (projectDir.rename(oldDirName, projectDir.path())) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - Directory renamed to -> " << projectDir.path();
                #endif
            } else {
                // If rename action fails, then try to create new project's path
                if (!projectDir.mkdir(projectDir.path())) {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupFileManager::save()] - Error: Can't create path -> " << projectDir.path();
                    #endif
                    return false;
                } else {
                    #ifdef TUP_DEBUG
                        qDebug() << "[TupFileManager::save()] - Directory was created successfully -> " << projectDir.path();
                    #endif
                }
            }
        }
    } else {
        // If project's path doesn't exist, create it
        if (!projectDir.exists()) {
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Creating project's directory...";
            #endif
            if (!projectDir.mkdir(projectDir.path())) {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupFileManager::save()] - Error: Can't create path -> " << projectDir.path();
                #endif
                return false;
            } else {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - Directory was created successfully -> " << projectDir.path();
                #endif
            }
        }
    }

    {
     #ifdef TUP_DEBUG
         qDebug() << "[TupFileManager::save()] - source files path -> " << projectDir.path();
     #endif

     // Save project
     QFile projectFile(projectDir.path() + "/project.tpp");

     if (projectFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
         #ifdef TUP_DEBUG
             qDebug() << "[TupFileManager::save()] - Saving config file (TPP)";
         #endif
         QTextStream ts(&projectFile);
         QDomDocument doc;
         project->setProjectName(name);
         doc.appendChild(project->toXml(doc));
         ts << doc.toString();
         projectFile.close();
     } else {
         #ifdef TUP_DEBUG
             qWarning() << "[TupFileManager::save()] - Error: Can't create file -> " << projectDir.path() << "/project.tpp";
         #endif
         return false;
     }
    }

    // Save scenes
    {
        int totalScenes = project->getScenes().size();
        int index = 0;
        QDomDocument doc;
        QString scenePath;

        for (int i=0; i<totalScenes; i++) {
            doc.appendChild(project->getScenes().at(i)->toXml(doc));
            scenePath = projectDir.path() + "/scene" + QString::number(index) + ".tps";
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Saving scene file " << i;
                qDebug() << "[TupFileManager::save()] - Scene file -> " << scenePath;
            #endif
            QFile sceneFile(scenePath);
            if (sceneFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupFileManager::save()] - Capturing stream...";
                #endif
                QTextStream st(&sceneFile);
                st << doc.toString();
                index += 1;
                doc.clear();
                sceneFile.close();
            } else {
                #ifdef TUP_DEBUG
                    qWarning() << "[TupFileManager::save()] - Error: Can't create file -> " << scenePath;
                #endif
                return false;
            }
        }
    }

    {
         // Save library
         QFile library(projectDir.path() + "/library.tpl");

         if (library.open(QIODevice::WriteOnly | QIODevice::Text)) {
             #ifdef TUP_DEBUG
                 qDebug() << "[TupFileManager::save()] - Saving library file (TPL)";
             #endif
             QTextStream ts(&library);
             QDomDocument doc;
             doc.appendChild(project->getLibrary()->toXml(doc));

             ts << doc.toString();
             library.close();
         } else {
             #ifdef TUP_DEBUG
                 qWarning() << "[TupFileManager::save()] - Error: Can't create file -> " + projectDir.path() + "/library.tpl";
             #endif
             return false;
         }
    }

    {
        #ifdef TUP_DEBUG
            qDebug() << "[TupFileManager::save()] - Creating TUP file...";
        #endif
        TupPackageHandler packageHandler;
        ok = packageHandler.makePackage(projectDir.path(), fileName);

        if (ok) {
            #ifdef TUP_DEBUG
                qWarning() << "[TupFileManager::save()] - Project saved at -> " << fileName;
            #endif
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::save()] - Error: Project couldn't be saved at -> " << fileName;
            #endif

            QApplication::restoreOverrideCursor();
            QScreen *screen = QGuiApplication::screens().at(0);
            TBackupDialog *dialog = new TBackupDialog(projectDir.path(), name);
            dialog->show();

            int result = dialog->exec();
            if (result == QDialog::Accepted) {
                TCONFIG->beginGroup("General");
                QString path = TCONFIG->value("RecoveryDir").toString();
                TCONFIG->sync();

                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Information"));
                msgBox.setIcon(QMessageBox::Information);
                msgBox.setText(tr("A copy of your project was successfully saved at:<br/><b>%1</b>").arg(path));
                msgBox.setInformativeText(tr("Please, contact TupiTube's developer team to recovery it. "
                                             "<b>https://www.tupitube.com</b>"));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.show();
                msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                            static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));
                result = msgBox.exec();
            } else if (result == QDialog::Rejected) {
                QMessageBox msgBox;
                msgBox.setWindowTitle(tr("Information"));
                msgBox.setIcon(QMessageBox::Critical);
                msgBox.setText(tr("Sorry, the project <b>%1.tup</b> couldn't be recovered.<br/>"
                                  "Please, try to backup your animation files often.").arg(name));
                msgBox.setStandardButtons(QMessageBox::Ok);
                msgBox.show();
                msgBox.move(static_cast<int> ((screen->geometry().width() - msgBox.width()) / 2),
                            static_cast<int> ((screen->geometry().height() - msgBox.height()) / 2));
                result = msgBox.exec();
            }

            return false;
        }
    }

    return ok;
}

bool TupFileManager::load(const QString &fileName, TupProject *project)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupFileManager::load()] - fileName -> " << fileName;
    #endif

    TupPackageHandler packageHandler;
    if (packageHandler.importPackage(fileName)) {
        QDir projectDir(packageHandler.importedProjectPath());
        QFile pfile(projectDir.path() + "/project.tpp");

        if (pfile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            project->fromXml(QString::fromLocal8Bit(pfile.readAll()));
            pfile.close();
        } else {
            #ifdef TUP_DEBUG
                qWarning() << "[TupFileManager::load()] - Error while open .tpp file. Name: " << pfile.fileName();
                qWarning() << "[TupFileManager::load()] - Path: " << projectDir.path();
                qWarning() << "[TupFileManager::load()] - Error Description: " << pfile.errorString();
            #endif
            return false;
        }

        project->setDataDir(packageHandler.importedProjectPath());
        project->loadLibrary(projectDir.path() + "/library.tpl");

        QStringList scenes = projectDir.entryList(QStringList() << "*.tps", QDir::Readable | QDir::Files);
        QFile *file;
        QDomDocument doc;
        QString xml;
        QDomElement root;

        if (scenes.count() > 0) {
            int index = 0;
            foreach (QString scenePath, scenes) {
                scenePath = projectDir.path() + "/" + scenePath;
                file = new QFile(scenePath);
					 
                if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
                    xml = QString::fromLocal8Bit(file->readAll());
                    if (!doc.setContent(xml))
                        return false;

                    root = doc.documentElement();
                    project->createScene(root.attribute("name"), index, true)->fromXml(xml);
                    index += 1;
                    doc.clear();
                    file->close();
                    delete file;
                } else {
                    #ifdef TUP_DEBUG
                        qWarning() << "[TupFileManager::load()] - Error: Can't open file -> " << scenePath;
                    #endif
                    return false;
                }
            }
            project->setOpen(true);
            return true;
        } else {
            #ifdef TUP_DEBUG
                qDebug() << "[TupFileManager::load()] - Error: No scene files found (*.tps)";
            #endif
            return false;
        }
    }

    #ifdef TUP_DEBUG
        qDebug() << "[TupFileManager::load()] - Error: Can't import package -> " << fileName;
    #endif
    return false;
}

bool TupFileManager::createImageProject(const QString &projectCode, const QString &imgPath, TupProject *project)
{
    #ifdef TUP_DEBUG
        qDebug() << "[TupFileManager::createImageProject()] - projectCode -> " << projectCode;
        qDebug() << "[TupFileManager::createImageProject()] - imgPath -> " << imgPath;
    #endif

    QString filename = CACHE_DIR + projectCode + ".tup";

    TupProject *newProject = new TupProject();
    newProject->setProjectName(projectCode);
    newProject->setAuthor(project->getAuthor());
    // newProject->setTags(project->getTags());
    newProject->setBgColor(project->getBgColor());
    newProject->setDescription(project->getDescription());
    newProject->setDimension(project->getDimension());
    newProject->setFPS(project->getFPS(), 0);
    newProject->setDataDir(CACHE_DIR + projectCode);

    TupLibrary *library = new TupLibrary("library", newProject);
    newProject->setLibrary(library);

    TupScene * newScene = newProject->createScene(tr("Scene %1").arg(QString::number(1)), 0);
    TupLayer *newLayer = newScene->createLayer(tr("Layer %1").arg(QString::number(1)), 0);
    newLayer->createFrame(tr("Frame %1").arg(QString::number(1)), 0);
    TupFrame *frame = newLayer->frameAt(0);

    QFile file(imgPath);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            if (library->createSymbol(TupLibraryObject::Image, "image.png", data, "") == nullptr) {
                #ifdef TUP_DEBUG
                    qDebug() << "[TupProject::createImageProject()] - Fatal error: image object can't be created. Data is NULL!";
                #endif
                return false;
            }

            TupLibraryObject *object = library->getObject("image.png");
            if (object) {
                TupGraphicLibraryItem *libraryItem = new TupGraphicLibraryItem(object);
                int imageW = static_cast<int>(libraryItem->boundingRect().width());
                int imageH = static_cast<int> (libraryItem->boundingRect().height());

                qreal xPos = 0;
                qreal yPos = 0;
                QSize dimension = newProject->getDimension();
                if (dimension.width() > imageW)
                    xPos = (dimension.width() - imageW) / 2;
                if (dimension.height() > imageH)
                    yPos = (dimension.height() - imageH) / 2;

                libraryItem->moveBy(xPos, yPos);

                int zLevel = frame->getTopZLevel();
                libraryItem->setZValue(zLevel);
                frame->addItem("image.png", libraryItem);
            }
        }
    }

    return save(filename, newProject);
}
