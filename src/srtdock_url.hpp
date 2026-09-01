#pragma once

#include <QString>

struct SRTDockUrlParts
{
    QString host;
    int port = 9000;
    int latencyMs = 120;
    QString mode = QStringLiteral("caller");
    QString passphrase;
    QString streamId;
};

class SRTDockUrl
{
public:
    static QString build(const SRTDockUrlParts &parts);
};
