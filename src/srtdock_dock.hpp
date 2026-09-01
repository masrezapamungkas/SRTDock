#pragma once

#include <QWidget>

class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QToolButton;
class QTimer;
class QLayout;

class SRTDockWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SRTDockWidget(QWidget *parent = nullptr);

private slots:
    void updateUrl();
    void updateModeUi(const QString &mode);
    void copyListenerLink(const QString &link);
    void copyCallerEndpoint(const QString &endpoint);

private:
    void buildUi();
    QString buildUrl() const;
    QString buildUrlForHost(const QString &host) const;
    QString listenerHost() const;
    bool vpnConnected() const;
    void clearDynamicRows(QLayout *layout);

    QLineEdit *hostEdit;
    QSpinBox *portSpin;
    QSpinBox *latencySpin;
    QComboBox *modeCombo;
    QCheckBox *passphraseCheck;
    QLineEdit *passphraseEdit;
    QCheckBox *streamIdCheck;
    QLineEdit *streamIdEdit;
    QLabel *errorLabel;
    QLabel *listenerInfoLabel;
    QLabel *callerModeHintLabel;
    QLineEdit *callerModeUrlEdit;
    QToolButton *callerModeCopyButton;
    QWidget *callerModeWidget;
    QTimer *networkRefreshTimer;
    QLabel *mediaSourceHintLabel;
    QVBoxLayout *mediaSourceLayout;
    QLabel *callerEndpointHintLabel;
    QComboBox *callerEndpointTypeCombo;
    QVBoxLayout *callerEndpointLayout;
    QWidget *callerEndpointWidget;
    QFormLayout *form;
    QVBoxLayout *listenerButtonsLayout;
    QString localCallerEndpoint;
    QString vpnCallerEndpoint;
};
