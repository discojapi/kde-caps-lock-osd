/*
    SPDX-FileCopyrightText: 2014 Martin Klapetek <mklapetek@kde.org>
    SPDX-FileCopyrightText: 2024 Jakob Petsovits <jpetso@petsovits.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "osd.h"
#include "debug.h"
#include "shellcorona.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QTimer>
#include <QWindow>

#include <PlasmaQuick/SharedQmlEngine>
#include <klocalizedstring.h>

#include <algorithm> // std::ranges::sort

using namespace Qt::StringLiterals;

Osd::Osd(const KSharedConfig::Ptr &config, ShellCorona *corona)
    : QObject(corona)
    , m_corona(corona)
    , m_osdConfigGroup(config, u"OSD"_s)
{
    QDBusConnection::sessionBus().registerObject(u"/org/kde/osdService"_s, this, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
}

Osd::~Osd()
{
}
Osd::capsLockEnabledChanged(bool capsLockEnabled){
    if (capsLockEnabled) {
        showText(u"input-touchpad-on"_s, i18nc("touchpad was enabled, keep short", "Touchpad On"));
    } else {
        showText(u"input-touchpad-off"_s, i18nc("touchpad was disabled, keep short", "Touchpad Off"));
    }

}
Osd::numLockEnabledChanged(bool numLockEnabled){
    if (numLockEnabled) {
        showText(u"input-touchpad-on"_s, i18nc("touchpad was enabled, keep short", "Touchpad On"));
    } else {
        showText(u"input-touchpad-off"_s, i18nc("touchpad was disabled, keep short", "Touchpad Off"));
    }
}
bool Osd::init()
{
    if (!m_osdConfigGroup.readEntry("Enabled", true)) {
        return false;
    }

    if (m_osdObject && m_osdObject->rootObject()) {
        return true;
    }

    const QUrl url = m_corona->kPackage().fileUrl("osdmainscript");
    if (url.isEmpty()) {
        return false;
    }

    if (!m_osdObject) {
        m_osdObject = new PlasmaQuick::SharedQmlEngine(this);
    }

    m_osdObject->setSource(url);

    if (m_osdObject->status() != QQmlComponent::Ready) {
        qCWarning(PLASMASHELL) << "Failed to load OSD QML file" << url;
        auto fallbackUrl = m_corona->kPackage().fallbackPackage().fileUrl("osdmainscript");
        if (fallbackUrl.isEmpty() || fallbackUrl == url) {
            return false;
        }
        qCWarning(PLASMASHELL) << "Trying fallback theme";
        m_osdObject->setSource(fallbackUrl);
        if (m_osdObject->status() != QQmlComponent::Ready) {
            qCWarning(PLASMASHELL) << "Failed to load fallback OSD QML file" << fallbackUrl;
            return false;
        }
    }

    m_timeout = m_osdObject->rootObject()->property("timeout").toInt();

    if (!m_osdTimer) {
        m_osdTimer = new QTimer(this);
        m_osdTimer->setSingleShot(true);
        connect(m_osdTimer, &QTimer::timeout, this, &Osd::hideOsd);
    }

    return true;
}

void Osd::showText(const QString &icon, const QString &text)
{
    if (!init()) {
        return;
    }

    auto *rootObject = m_osdObject->rootObject();

    rootObject->setProperty("showingProgress", false);
    rootObject->setProperty("osdValue", text);
    rootObject->setProperty("icon", icon);

    Q_EMIT osdText(icon, text);
    showOsd();
}

void Osd::showOsd()
{
    m_osdTimer->stop();

    auto *rootObject = m_osdObject->rootObject();

    // if our OSD understands animating the opacity, do it;
    // otherwise just show it to not break existing lnf packages
    if (rootObject->property("animateOpacity").isValid()) {
        rootObject->setProperty("animateOpacity", false);
        rootObject->setProperty("opacity", 1);
        rootObject->setProperty("visible", true);
        rootObject->setProperty("animateOpacity", true);
        rootObject->setProperty("opacity", 0);
    } else {
        rootObject->setProperty("visible", true);
    }

    m_osdTimer->start(m_timeout);
}

void Osd::hideOsd()
{
    auto *rootObject = m_osdObject->rootObject();
    if (!rootObject) {
        return;
    }

    rootObject->setProperty("visible", false);

    // this is needed to prevent fading from "old" values when the OSD shows up
    rootObject->setProperty("osdValue", 0);

    m_screenBrightnessInfo.clear();
}

#include "moc_osd.cpp"
