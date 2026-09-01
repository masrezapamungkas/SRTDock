#include "srtdock_url.hpp"

#include <QStringList>

QString SRTDockUrl::build(const SRTDockUrlParts &parts)
{
    QString url = QStringLiteral("srt://%1:%2?mode=%3&latency=%4")
                      .arg(parts.host)
                      .arg(parts.port)
                      .arg(parts.mode)
                      .arg(parts.latencyMs * 1000);

    if (!parts.passphrase.isEmpty())
        url += QStringLiteral("&passphrase=%1").arg(parts.passphrase);

    if (!parts.streamId.isEmpty())
        url += QStringLiteral("&streamid=%1").arg(parts.streamId);


    return url;
}
