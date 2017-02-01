/*
 *   Copyright (C) 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMAVAULT_TESTAPP_WINDOW_H
#define PLASMAVAULT_TESTAPP_WINDOW_H

#include <QMainWindow>
#include <memory>

class ModelTest;
namespace Ui {
    class MainWindow;
}

class Window: public QMainWindow {
    Q_OBJECT

public:
    Window();
    ~Window();

public Q_SLOTS:
    void buttonInitializeClicked();
    void buttonOpenClicked();
    void buttonCloseClicked();
    void buttonDestroyClicked();
    void buttonLoadClicked();

    void updateStatus();
    void updateIsOpened();
    void updateIsInitialized();

private:
    std::unique_ptr<Ui::MainWindow> ui;

    class Private;
    Private * const d;

};

#endif // PLASMAVAULT_TESTAPP_WINDOW_H

