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

#include "window.h"

#include "ui_window.h"

#include <QTimer>

#include "vault.h"

#include <memory>

#include <asynqt/operations/continuewith.h>

class Window::Private {
public:
    Private()
        : busyProgress(0)
    {
    }

    std::unique_ptr<PlasmaVault::Vault> vault;
    QTimer timer;
    int busyProgress;
};

Window::Window()
    : ui(new Ui::MainWindow())
    , d(new Private())
{
    ui->setupUi(this);

    connect(ui->buttonInitialize, SIGNAL(clicked()),
            this, SLOT(buttonInitializeClicked()));
    connect(ui->buttonOpen, SIGNAL(clicked()),
            this, SLOT(buttonOpenClicked()));
    connect(ui->buttonClose, SIGNAL(clicked()),
            this, SLOT(buttonCloseClicked()));
    connect(ui->buttonDestroy, SIGNAL(clicked()),
            this, SLOT(buttonDestroyClicked()));
    connect(ui->buttonLoad, SIGNAL(clicked()),
            this, SLOT(buttonLoadClicked()));

    d->timer.setInterval(50);
    d->timer.setSingleShot(false);

    connect(&d->timer, &QTimer::timeout,
            this, [=] () {
                d->busyProgress++;
                if (d->busyProgress > 20) {
                    d->busyProgress = 0;
                }
            ui->busy->setValue(d->busyProgress);
            });

    ui->busy->setVisible(false);
    // d->timer.start();
}

Window::~Window()
{
    delete d;
}

template <typename T>
void executeCommand(const T &command)
{
    AsynQt::transform(
        command,
        [] (const PlasmaVault::Result<> &result) {
            // result.visit([] (const auto &result) {
            //     qDebug() << result;
            // });
        });
}

void Window::buttonInitializeClicked()
{
    executeCommand(
            d->vault->create(ui->textName->text(),
                             ui->textMountPoint->text(),
                             "somepassword",
                             "encfs")
        );
}

void Window::buttonOpenClicked()
{
    executeCommand(
            d->vault->open("somepassword")
        );
}

void Window::buttonCloseClicked()
{
    executeCommand(
            d->vault->close()
        );
}

void Window::buttonDestroyClicked()
{
}

void Window::buttonLoadClicked()
{
    d->vault.reset(new PlasmaVault::Vault(ui->textDevice->text()));

    connect(d->vault.get(), &PlasmaVault::Vault::mountPointChanged, this,
            [this] (const QString &mountPoint) {
                ui->textMountPoint->setText(mountPoint);
            }
        );

    connect(d->vault.get(), &PlasmaVault::Vault::statusChanged, this,
            [this] (PlasmaVault::Vault::Status status) {
                updateStatus();
            }
        );

    connect(d->vault.get(), &PlasmaVault::Vault::isOpenedChanged, this,
            [this] (bool is) {
                updateIsOpened();
            }
        );

    connect(d->vault.get(), &PlasmaVault::Vault::isInitializedChanged, this,
            [this] (bool is) {
                updateIsInitialized();
            }
        );

    ui->textMountPoint->setText(d->vault->mountPoint());
    ui->textName->setText(d->vault->name());
    updateStatus();
    updateIsOpened();
    updateIsInitialized();

    ui->panelOptions->setEnabled(true);
}

void Window::updateStatus()
{
    auto status = d->vault->status();
    QString text =
        status == PlasmaVault::Vault::Error          ? "Error" :

        status == PlasmaVault::Vault::NotInitialized ? "NotInitialized" :
        status == PlasmaVault::Vault::Opened         ? "Opened" :
        status == PlasmaVault::Vault::Closed         ? "Closed" :

        status == PlasmaVault::Vault::Creating       ? "Creating" :
        status == PlasmaVault::Vault::Opening        ? "Opening" :
        status == PlasmaVault::Vault::Closing        ? "Closing" :
        status == PlasmaVault::Vault::Destroying     ? "Destroying" :
                                (QString::number(status) + " (status does not exist)");

    ui->labelStatus->setText("Status: " + text);
    ui->labelError->setText(d->vault->errorMessage());

    bool isBusy =
        status == PlasmaVault::Vault::Error          ? false :

        status == PlasmaVault::Vault::NotInitialized ? false :
        status == PlasmaVault::Vault::Opened         ? false :
        status == PlasmaVault::Vault::Closed         ? false :

        status == PlasmaVault::Vault::Creating       ? true :
        status == PlasmaVault::Vault::Opening        ? true :
        status == PlasmaVault::Vault::Closing        ? true :
        status == PlasmaVault::Vault::Destroying     ? true : false;

    ui->busy->setVisible(isBusy);
    if (isBusy) {
        d->timer.start();
    } else {
        d->timer.stop();
    }
}

void Window::updateIsOpened()
{
    ui->checkMounted->setChecked(d->vault->isOpened());
}

void Window::updateIsInitialized()
{
    ui->checkInitialized->setChecked(d->vault->isInitialized());
}

