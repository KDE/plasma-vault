/*
 *   Copyright (C) 2017 Ivan Čukić <ivan.cukic(at)kde.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) version 3, or any
 *   later version accepted by the membership of KDE e.V. (or its
 *   successor approved by the membership of KDE e.V.), which shall
 *   act as a proxy defined in Section 6 of version 3 of the license.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.
 *   If not, see <http://www.gnu.org/licenses/>.
 */

#include "dialogdsl.h"

#include <QVBoxLayout>

namespace DialogDsl {



Key::Key(const QByteArray &key, const QString &translation)
    : QByteArray(key)
    , m_translation(translation)
{
}



QString Key::translation() const
{
    return m_translation;
}



DialogModule::DialogModule(bool valid)
    : m_isValid(valid)
{
}



void DialogModule::setIsValid(bool valid)
{
    if (valid == m_isValid) return;

    m_isValid = valid;

    emit isValidChanged(valid);
}



bool DialogModule::isValid() const
{
    return m_isValid;
}



void DialogModule::init(const PlasmaVault::Vault::Payload &payload)
{
}



CompoundDialogModule::CompoundDialogModule(const step &children)
    : DialogDsl::DialogModule(false)
{
    auto layout = new QVBoxLayout(this);
    setLayout(layout);

    bool valid = true;

    for (const auto& childFactory: children) {
        auto child = childFactory();
        child->setParent(this);
        child->setSizePolicy(
                QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
        m_children << child;

        valid &= child->isValid();

        layout->addWidget(child);
    }

    setIsValid(valid);

    auto spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout->addSpacerItem(spacer);
}



PlasmaVault::Vault::Payload CompoundDialogModule::fields() const
{
    PlasmaVault::Vault::Payload result;

    for (const auto& child: m_children) {
        result.unite(child->fields());
    }

    return result;
}



void CompoundDialogModule::init(const PlasmaVault::Vault::Payload &payload)
{
    for (const auto& child: m_children) {
        child->init(payload);
    }
}

} // namespace DialogDsl

