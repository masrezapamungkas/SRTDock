#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QMainWindow>

#include "srtdock_dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("SRTDock", "en-US")

static SRTDockWidget *dockWidget = nullptr;

bool obs_module_load(void)
{
    QMainWindow *mainWindow = (QMainWindow *)obs_frontend_get_main_window();
    if (!mainWindow)
        return false;

    dockWidget = new SRTDockWidget(mainWindow);
    obs_frontend_add_dock_by_id("SRTDock", "SRTDock", dockWidget);

    blog(LOG_INFO, "SRTDock plugin loaded successfully");
    return true;
}

void obs_module_unload(void)
{
    blog(LOG_INFO, "SRTDock plugin unloaded");
}
