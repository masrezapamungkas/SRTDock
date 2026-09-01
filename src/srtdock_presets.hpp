#pragma once

#include <QString>

struct SRTDockPreset
{
    QString name;
    QString host;
    int port = 9000;
    int latencyMs = 120;
    QString mode = QStringLiteral("caller");
    QString passphrase;
    QString streamId;
};
