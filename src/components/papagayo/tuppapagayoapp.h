/***************************************************************************
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

#ifndef TUPPAPAGAYOAPP_H
#define TUPPAPAGAYOAPP_H

#include <QMainWindow>
#include <QLineEdit>
#include <QListWidgetItem>
#include <QPlainTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QStackedWidget>
// #include <QSpinBox>

#include "tglobal.h"
#include "tuplipsyncdoc.h"
#include "tupwaveformview.h"
#include "tupmouthview.h"
#include "tupcustomizedmouthview.h"

class Q_DECL_EXPORT TupPapagayoApp : public QMainWindow
{
	Q_OBJECT

    public:
        enum ViewType { Predefined = 0, Customized };
        explicit TupPapagayoApp(bool extendedUI, int32 fps, const QString &soundFile = QString(), QWidget *parent = nullptr);
        ~TupPapagayoApp();

    public:
        void openFile(QString filePath);
        bool isOKToCloseDocument();

    protected:
        void closeEvent(QCloseEvent *event);
        void dragEnterEvent(QDragEnterEvent *event);
        void dropEvent(QDropEvent *event);
        void keyPressEvent(QKeyEvent *event);

    public slots:
        void updateActions();

    private slots:
        void onFileOpen();
        void onFileSave();
        void onPlay();
        void onPause();
        void onStop();
        void onFpsChange(int fps);
        void onVoiceNameChanged();
        void onVoiceTextChanged();
        void onBreakdown();
        void updateMouthView(int index);
        void openImagesDialog();
        void updateFrame(int frame);
        void updatePauseButton();
        void createLipsyncRecord();

    private:
        void setBaseUI();
        void setUIStyle();
        void setupActions();
        void setupUI();
        void setupMenus();
        void saveDefaultPath(const QString &dir);

        TupWaveFormView *waveformView;
        TupMouthView *mouthView;
        TupCustomizedMouthView *customView;
        TupLipsyncDoc *document;
        bool enableAutoBreakdown;
        bool rebuildingList;
        int defaultFps;
        bool playerStopped;
        bool extendedUI;

        QAction *actionClose;
        QAction *actionOpen;
        QAction *actionSave;
        QAction *actionPlay;
        QIcon playIcon;
        QIcon pauseIcon;

        QAction *actionStop;
        QAction *actionZoomIn;
        QAction *actionZoomOut;
        QAction *actionAutoZoom;
        QAction *actionUndo;
        QAction *actionCut;
        QAction *actionCopy;
        QAction *actionPaste;

        QStackedWidget *mouthFrame;
        QLineEdit *voiceName;
        QPlainTextEdit *voiceText;
        // QLineEdit *fpsEdit;
        // QSpinBox *fpsEdit;
        QComboBox *mouthsCombo;
        QComboBox *languageChoice;
        QPushButton *breakdownButton;
        QLineEdit *mouthsPath;
        QPushButton *okButton;
};

#endif // TUPPAPAGAYODIALOG_H
