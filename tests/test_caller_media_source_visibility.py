from pathlib import Path

SOURCE = Path(__file__).parents[1] / "src" / "srtdock_dock.cpp"


def test_caller_media_source_widget_requires_host():
    text = SOURCE.read_text(encoding="utf-8")
    assert "const bool hasHost = !hostEdit->text().trimmed().isEmpty();" in text
    assert "callerModeWidget->setVisible(caller && hasHost);" in text


def test_caller_media_source_url_is_not_generated_without_host():
    text = SOURCE.read_text(encoding="utf-8")
    assert "if (caller && hasHost)" in text


if __name__ == "__main__":
    for name, fn in sorted(globals().items()):
        if name.startswith("test_"):
            fn()
    print("caller media source visibility checks passed")
