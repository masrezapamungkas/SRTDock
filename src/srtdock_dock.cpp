#include "srtdock_dock.hpp"

#include "srtdock_url.hpp"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDockWidget>
#include <QLayout>

SRTDockWidget::SRTDockWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(320);
    buildUi();
    updateUrl();
}

void SRTDockWidget::buildUi()
{
    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(4);
    root->setAlignment(Qt::AlignTop);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    form = new QFormLayout();
    form->setVerticalSpacing(3);

    hostEdit = new QLineEdit(this);
    hostEdit->setPlaceholderText(QStringLiteral("192.168.1.10"));
    connect(hostEdit, &QLineEdit::textChanged, this, &SRTDockWidget::updateUrl);

    portSpin = new QSpinBox(this);
    portSpin->setRange(1, 65535);
    portSpin->setValue(9000);
    connect(portSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SRTDockWidget::updateUrl);
    connect(portSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int) {
        updateModeUi(modeCombo->currentText());
    });

    latencySpin = new QSpinBox(this);
    latencySpin->setRange(1, 60000);
    latencySpin->setValue(120);
    latencySpin->setSuffix(QStringLiteral(" ms"));
    connect(latencySpin, QOverload<int>::of(&QSpinBox::valueChanged), this, &SRTDockWidget::updateUrl);

    modeCombo = new QComboBox(this);
    modeCombo->addItems({QStringLiteral("Caller"), QStringLiteral("Listener"), QStringLiteral("Rendezvous")});
    connect(modeCombo, &QComboBox::currentTextChanged, this, &SRTDockWidget::updateUrl);
    connect(modeCombo, &QComboBox::currentTextChanged, this, &SRTDockWidget::updateModeUi);

    passphraseCheck = new QCheckBox(QStringLiteral("Passphrase"), this);
    passphraseEdit = new QLineEdit(this);
    passphraseEdit->setEchoMode(QLineEdit::Password);
    passphraseEdit->setPlaceholderText(QStringLiteral("(optional)"));
    passphraseEdit->setEnabled(false);
    connect(passphraseCheck, &QCheckBox::toggled, passphraseEdit, &QWidget::setEnabled);
    connect(passphraseEdit, &QLineEdit::textChanged, this, &SRTDockWidget::updateUrl);
    connect(passphraseCheck, &QCheckBox::toggled, this, &SRTDockWidget::updateUrl);

    streamIdCheck = new QCheckBox(QStringLiteral("Stream ID"), this);
    streamIdEdit = new QLineEdit(this);
    streamIdEdit->setPlaceholderText(QStringLiteral("(optional)"));
    streamIdEdit->setEnabled(false);
    connect(streamIdCheck, &QCheckBox::toggled, streamIdEdit, &QWidget::setEnabled);
    connect(streamIdEdit, &QLineEdit::textChanged, this, &SRTDockWidget::updateUrl);
    connect(streamIdCheck, &QCheckBox::toggled, this, &SRTDockWidget::updateUrl);


    form->addRow(QStringLiteral("Mode"), modeCombo);
    form->addRow(QStringLiteral("Host / IP"), hostEdit);
    form->addRow(QStringLiteral("Port"), portSpin);
    form->addRow(QStringLiteral("Latency"), latencySpin);
    form->addRow(passphraseCheck, passphraseEdit);
    form->addRow(streamIdCheck, streamIdEdit);

    root->addLayout(form);

    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet(QStringLiteral("color: #cc4444; font-size: 11px;"));
    errorLabel->hide();
    root->addWidget(errorLabel);

    listenerInfoLabel = new QLabel(this);
    listenerInfoLabel->setWordWrap(true);
    listenerInfoLabel->setStyleSheet(QStringLiteral("color: #448844; font-size: 16px; font-weight: bold; padding: 8px 0;"));
    root->addWidget(listenerInfoLabel);

    callerModeWidget = new QWidget(this);
    auto *callerModeLayout = new QVBoxLayout(callerModeWidget);
    callerModeLayout->setContentsMargins(0, 0, 0, 0);
    auto *callerModeRow = new QWidget(callerModeWidget);
    auto *callerModeRowLayout = new QHBoxLayout(callerModeRow);
    callerModeRowLayout->setContentsMargins(0, 0, 0, 0);
    callerModeUrlEdit = new QLineEdit(callerModeRow);
    callerModeUrlEdit->setReadOnly(true);
    callerModeUrlEdit->setMinimumWidth(0);
    callerModeUrlEdit->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    callerModeRowLayout->addWidget(callerModeUrlEdit, 1);
    callerModeCopyButton = new QToolButton(callerModeRow);
    callerModeCopyButton->setText(QStringLiteral("⧉"));
    callerModeCopyButton->setToolTip(QStringLiteral("Copy caller URL to share with the receiving listener"));
    callerModeCopyButton->setFixedSize(28, 28);
    connect(callerModeCopyButton, &QToolButton::clicked, this, [this]() {
        copyCallerEndpoint(callerModeUrlEdit->text());
    });
    callerModeRowLayout->addWidget(callerModeCopyButton);
    callerModeLayout->addWidget(callerModeRow);
    callerModeHintLabel = new QLabel(QStringLiteral("Caller URL - share this endpoint with the receiving listener. Do not paste it into OBS Media Source."), callerModeWidget);
    callerModeHintLabel->setWordWrap(true);
    callerModeHintLabel->setStyleSheet(QStringLiteral("color: #a9b7c6; font-size: 11px;"));
    callerModeLayout->addWidget(callerModeHintLabel);
    callerModeWidget->setVisible(false);
    root->addWidget(callerModeWidget);

    mediaSourceHintLabel = new QLabel(QStringLiteral("OBS Media Source URL"), this);
    mediaSourceHintLabel->setStyleSheet(QStringLiteral("color: #a9b7c6; font-size: 12px; font-weight: bold; padding-top: 8px;"));
    mediaSourceHintLabel->setVisible(false);
    root->addWidget(mediaSourceHintLabel);

    mediaSourceLayout = new QVBoxLayout();
    mediaSourceLayout->setContentsMargins(0, 0, 0, 0);
    mediaSourceLayout->setSpacing(3);
    mediaSourceLayout->setAlignment(Qt::AlignTop);
    root->addLayout(mediaSourceLayout);

    listenerButtonsLayout = new QVBoxLayout();
    listenerButtonsLayout->setContentsMargins(0, 0, 0, 0);
    listenerButtonsLayout->setSpacing(0);
    root->addLayout(listenerButtonsLayout);

    callerEndpointHintLabel = new QLabel(QStringLiteral("Caller Endpoint"), this);
    callerEndpointHintLabel->setStyleSheet(QStringLiteral("color: #a9b7c6; font-size: 12px; font-weight: bold; padding-top: 8px;"));
    callerEndpointHintLabel->setVisible(false);
    root->addWidget(callerEndpointHintLabel);

    callerEndpointTypeCombo = new QComboBox(this);
    callerEndpointTypeCombo->addItems({QStringLiteral("OBS"), QStringLiteral("Other Applications")});
    callerEndpointTypeCombo->setVisible(false);
    connect(callerEndpointTypeCombo, &QComboBox::currentTextChanged, this, [this](const QString &) {
        updateModeUi(modeCombo->currentText());
    });
    root->addWidget(callerEndpointTypeCombo);

    callerEndpointWidget = new QWidget(this);
    callerEndpointWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    callerEndpointLayout = new QVBoxLayout(callerEndpointWidget);
    callerEndpointLayout->setContentsMargins(0, 0, 0, 0);
    callerEndpointLayout->setSpacing(3);
    callerEndpointLayout->setAlignment(Qt::AlignTop);
    callerEndpointWidget->setVisible(false);
    root->addWidget(callerEndpointWidget);

    updateModeUi(modeCombo->currentText());
    networkRefreshTimer = new QTimer(this);
    networkRefreshTimer->setInterval(2000);
    connect(networkRefreshTimer, &QTimer::timeout, this, [this]() {
        if (modeCombo->currentText().compare(QStringLiteral("Listener"), Qt::CaseInsensitive) == 0)
            updateModeUi(modeCombo->currentText());
    });
    networkRefreshTimer->start();
}

QString SRTDockWidget::listenerHost() const
{
    QString fallback;
    for (const QNetworkInterface &interface : QNetworkInterface::allInterfaces()) {
        if (!(interface.flags() & QNetworkInterface::IsUp) ||
            !(interface.flags() & QNetworkInterface::IsRunning) ||
            (interface.flags() & QNetworkInterface::IsLoopBack))
            continue;
        for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
            const QHostAddress address = entry.ip();
            if (address.protocol() != QAbstractSocket::IPv4Protocol || address.isLoopback())
                continue;
            const QString value = address.toString();
            if (value.startsWith(QStringLiteral("100.")))
                return value;
            if (fallback.isEmpty())
                fallback = value;
        }
    }
    return fallback;
}

bool SRTDockWidget::vpnConnected() const
{
    QProcess tailscale;
    tailscale.start(QStringLiteral("C:/Program Files/Tailscale/tailscale.exe"),
                    {QStringLiteral("status"), QStringLiteral("--json")});
    if (tailscale.waitForFinished(1500) && tailscale.exitCode() == 0) {
        const QJsonDocument document = QJsonDocument::fromJson(tailscale.readAllStandardOutput());
        const QJsonObject root = document.object();
        const QJsonObject self = root.value(QStringLiteral("Self")).toObject();
        if (root.value(QStringLiteral("BackendState")).toString() == QStringLiteral("Running") &&
            self.value(QStringLiteral("Online")).toBool(false))
            return true;
    }
    QProcess zerotier;
    zerotier.start(QStringLiteral("zerotier-cli"), {QStringLiteral("info")});
    if (zerotier.waitForFinished(1500) && zerotier.exitCode() == 0 &&
        QString::fromUtf8(zerotier.readAllStandardOutput()).contains(QStringLiteral("ONLINE"), Qt::CaseInsensitive)) {
        QProcess networks;
        networks.start(QStringLiteral("zerotier-cli"), {QStringLiteral("listnetworks")});
        if (networks.waitForFinished(1500) && networks.exitCode() == 0 &&
            QString::fromUtf8(networks.readAllStandardOutput()).contains(QStringLiteral("OK")))
            return true;
    }
    return false;
}

void SRTDockWidget::updateModeUi(const QString &mode)
{
    const bool listener = mode.compare(QStringLiteral("Listener"), Qt::CaseInsensitive) == 0;
    if (hostEdit)
        hostEdit->setVisible(!listener);
    if (form && hostEdit && form->labelForField(hostEdit))
        form->labelForField(hostEdit)->setVisible(!listener);
    if (!listenerInfoLabel)
        return;
    if (!listener) {
        listenerInfoLabel->clear();
        clearDynamicRows(mediaSourceLayout);
        clearDynamicRows(callerEndpointLayout);
        clearDynamicRows(listenerButtonsLayout);
        const bool caller = mode.compare(QStringLiteral("Caller"), Qt::CaseInsensitive) == 0;
        const bool hasHost = !hostEdit->text().trimmed().isEmpty();
        callerModeWidget->setVisible(caller && hasHost);
        if (caller && hasHost)
            callerModeUrlEdit->setText(buildUrl());
        else
            callerModeUrlEdit->clear();
        mediaSourceHintLabel->setVisible(false);
        mediaSourceLayout->setEnabled(false);
        callerEndpointHintLabel->setVisible(false);
        callerEndpointTypeCombo->setVisible(false);
        callerEndpointWidget->setVisible(false);
        return;
    }

    callerModeWidget->setVisible(false);
    QString localLink;
    QString vpnLink;
    const bool vpnConnectedNow = vpnConnected();
    localCallerEndpoint.clear();
    vpnCallerEndpoint.clear();
    for (const QNetworkInterface &interface : QNetworkInterface::allInterfaces()) {
        if (!(interface.flags() & QNetworkInterface::IsUp) ||
            !(interface.flags() & QNetworkInterface::IsRunning) ||
            (interface.flags() & QNetworkInterface::IsLoopBack))
            continue;
        const QString interfaceName = (interface.humanReadableName() + interface.name()).toLower();
        const bool namedVpn = interfaceName.contains(QStringLiteral("tailscale")) ||
            interfaceName.contains(QStringLiteral("zerotier")) ||
            interfaceName.contains(QStringLiteral("zero tier"));
        for (const QNetworkAddressEntry &entry : interface.addressEntries()) {
            const QHostAddress address = entry.ip();
            if (address.protocol() != QAbstractSocket::IPv4Protocol || address.isLoopback())
                continue;
            const bool vpnAddress = namedVpn || address.toString().startsWith(QStringLiteral("100."));
            if (vpnAddress && !vpnConnectedNow)
                continue;
            const QString mediaLink = buildUrlForHost(address.toString());
            const QString callerEndpoint = callerEndpointTypeCombo->currentText() == QStringLiteral("OBS")
                ? buildUrlForHost(address.toString()).replace(QStringLiteral("mode=listener"), QStringLiteral("mode=caller"))
                : QStringLiteral("srt://%1:%2").arg(address.toString()).arg(portSpin->value());
            if (vpnAddress && vpnLink.isEmpty()) {
                vpnLink = mediaLink;
                vpnCallerEndpoint = callerEndpoint;
            } else if (!vpnAddress && localLink.isEmpty()) {
                localLink = mediaLink;
                localCallerEndpoint = callerEndpoint;
            }
        }
    }
    clearDynamicRows(listenerButtonsLayout);
    listenerInfoLabel->clear();
    mediaSourceHintLabel->setVisible(true);
    mediaSourceLayout->setEnabled(true);
    callerEndpointHintLabel->setVisible(true);
    callerEndpointTypeCombo->setVisible(true);
    clearDynamicRows(mediaSourceLayout);
    clearDynamicRows(callerEndpointLayout);

    auto addMediaRow = [this](QVBoxLayout *layout, const QString &description, const QString &link, const QString &tooltip) {
        auto *row = new QWidget(this);
        row->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto *descriptionLabel = new QLabel(description, row);
        descriptionLabel->setMinimumWidth(42);
        rowLayout->addWidget(descriptionLabel);
        auto *field = new QLineEdit(link, row);
        field->setReadOnly(true);
        field->setMinimumWidth(0);
        field->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        rowLayout->addWidget(field, 1);
        auto *button = new QToolButton(row);
        button->setText(QStringLiteral("⧉"));
        button->setToolTip(tooltip);
        button->setFixedSize(28, 28);
        connect(button, &QToolButton::clicked, this, [this, link]() { copyListenerLink(link); });
        rowLayout->addWidget(button);
        layout->addWidget(row);
    };
    if (!localLink.isEmpty())
        addMediaRow(mediaSourceLayout, QStringLiteral("Local"), localLink, QStringLiteral("Copy local OBS Media Source URL"));
    if (!vpnLink.isEmpty())
        addMediaRow(mediaSourceLayout, QStringLiteral("VPN"), vpnLink, QStringLiteral("Copy VPN OBS Media Source URL"));
    if (localLink.isEmpty() && vpnLink.isEmpty())
        listenerInfoLabel->setText(QStringLiteral("No reachable LAN/VPN IPv4 address detected."));

    if (!localCallerEndpoint.isEmpty()) {
        auto *row = new QWidget(callerEndpointWidget);
        row->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto *description = new QLabel(QStringLiteral("Local"), row);
        description->setMinimumWidth(42);
        rowLayout->addWidget(description);
        auto *label = new QLineEdit(localCallerEndpoint, row);
        label->setReadOnly(true);
        label->setMinimumWidth(0);
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        rowLayout->addWidget(label, 1);
        auto *button = new QToolButton(row);
        button->setText(QStringLiteral("⧉"));
        button->setToolTip(QStringLiteral("Copy local caller endpoint"));
        button->setFixedSize(28, 28);
        connect(button, &QToolButton::clicked, this, [this]() { copyCallerEndpoint(localCallerEndpoint); });
        rowLayout->addWidget(button);
        callerEndpointLayout->addWidget(row);
    }
    if (!vpnCallerEndpoint.isEmpty()) {
        auto *row = new QWidget(callerEndpointWidget);
        row->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        auto *rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(0, 0, 0, 0);
        auto *description = new QLabel(QStringLiteral("VPN"), row);
        description->setMinimumWidth(42);
        rowLayout->addWidget(description);
        auto *label = new QLineEdit(vpnCallerEndpoint, row);
        label->setReadOnly(true);
        label->setMinimumWidth(0);
        label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        rowLayout->addWidget(label, 1);
        auto *button = new QToolButton(row);
        button->setText(QStringLiteral("⧉"));
        button->setToolTip(QStringLiteral("Copy VPN caller endpoint"));
        button->setFixedSize(28, 28);
        connect(button, &QToolButton::clicked, this, [this]() { copyCallerEndpoint(vpnCallerEndpoint); });
        rowLayout->addWidget(button);
        callerEndpointLayout->addWidget(row);
    }
    callerEndpointWidget->setVisible(!localCallerEndpoint.isEmpty() || !vpnCallerEndpoint.isEmpty());
    if (layout()) {
        layout()->invalidate();
        layout()->activate();
    }
    updateGeometry();
    QTimer::singleShot(0, this, [this]() {
        QWidget *ancestor = parentWidget();
        while (ancestor && !qobject_cast<QDockWidget *>(ancestor))
            ancestor = ancestor->parentWidget();
        if (auto *dock = qobject_cast<QDockWidget *>(ancestor); dock && dock->isFloating())
            dock->adjustSize();
    });
}

QString SRTDockWidget::buildUrlForHost(const QString &host) const
{
    SRTDockUrlParts parts;
    parts.host = host;
    parts.port = portSpin->value();
    parts.latencyMs = latencySpin->value();
    parts.mode = modeCombo->currentText().toLower();
    if (passphraseCheck->isChecked())
        parts.passphrase = passphraseEdit->text().trimmed();
    if (streamIdCheck->isChecked())
        parts.streamId = streamIdEdit->text().trimmed();

    return SRTDockUrl::build(parts);
}

QString SRTDockWidget::buildUrl() const
{
    QString host = hostEdit->text().trimmed();
    if (modeCombo->currentText().compare(QStringLiteral("Listener"), Qt::CaseInsensitive) == 0)
        host = listenerHost();
    return buildUrlForHost(host);
}

void SRTDockWidget::updateUrl()
{
    const QString host = hostEdit->text().trimmed();
    if (host.isEmpty() && modeCombo->currentText().compare(QStringLiteral("Listener"), Qt::CaseInsensitive) != 0) {
        errorLabel->setText(QStringLiteral("Host / IP is required"));
        errorLabel->show();
        updateModeUi(modeCombo->currentText());
        return;
    }

    errorLabel->hide();
    if (callerModeWidget && callerModeWidget->isVisible())
        callerModeUrlEdit->setText(buildUrl());
    updateModeUi(modeCombo->currentText());
}

void SRTDockWidget::clearDynamicRows(QLayout *layout)
{
    if (!layout)
        return;
    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget()) {
            item->widget()->setParent(nullptr);
            delete item->widget();
        }
        delete item;
    }
}

void SRTDockWidget::copyListenerLink(const QString &link)
{
    QApplication::clipboard()->setText(link);
}

void SRTDockWidget::copyCallerEndpoint(const QString &endpoint)
{
    QApplication::clipboard()->setText(endpoint);
}
