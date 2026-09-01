from pathlib import Path

SOURCE = Path(__file__).parents[1] / "src" / "srtdock_dock.cpp"


def test_caller_validation_uses_host_ip_label():
    text = SOURCE.read_text(encoding="utf-8")
    assert 'QStringLiteral("Host / IP is required")' in text


def test_non_listener_mode_clears_dynamic_listener_rows():
    text = SOURCE.read_text(encoding="utf-8")
    non_listener = text[text.index('if (!listener) {'):text.index('if (!listener) {') + 1000]
    assert 'clearDynamicRows(mediaSourceLayout)' in non_listener
    assert 'clearDynamicRows(callerEndpointLayout)' in non_listener
    assert 'mediaSourceHintLabel->setVisible(false);' in non_listener
    assert 'callerEndpointWidget->setVisible(false);' in text


def test_dynamic_rows_are_cleared_before_listener_rebuild():
    text = SOURCE.read_text(encoding="utf-8")
    assert text.count('clearDynamicRows(mediaSourceLayout)') >= 2
    assert text.count('clearDynamicRows(callerEndpointLayout)') >= 2


def test_clear_helper_is_declared_and_defined():
    header = SOURCE.with_name("srtdock_dock.hpp").read_text(encoding="utf-8")
    text = SOURCE.read_text(encoding="utf-8")
    assert 'void clearDynamicRows(QLayout *layout);' in header
    assert 'void SRTDockWidget::clearDynamicRows(QLayout *layout)' in text
    assert '#include <QLayout>' in text


if __name__ == "__main__":
    for name, fn in sorted(globals().items()):
        if name.startswith("test_"):
            fn()
    print("all regression checks passed")
