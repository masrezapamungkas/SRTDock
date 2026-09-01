from pathlib import Path

ROOT = Path(__file__).parents[1]
DOCK = (ROOT / "src" / "srtdock_dock.cpp").read_text(encoding="utf-8")
PLUGIN = (ROOT / "src" / "plugin-main.cpp").read_text(encoding="utf-8")


def test_caller_media_source_is_hidden_without_host():
    assert "const bool hasHost = !hostEdit->text().trimmed().isEmpty();" in DOCK
    assert "callerModeWidget->setVisible(caller && hasHost);" in DOCK
    assert "if (caller && hasHost)" in DOCK


def test_plugin_registers_one_obs_managed_dock():
    assert "obs_frontend_add_dock_by_id(\"SRTDock\", \"SRTDock\", dockWidget);" in PLUGIN
    assert "new QDockWidget" not in PLUGIN
    assert "mainWindow->addDockWidget" not in PLUGIN


if __name__ == "__main__":
    for name, fn in sorted(globals().items()):
        if name.startswith("test_"):
            fn()
    print("startup and visibility checks passed")
