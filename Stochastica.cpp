#include "SC_PlugIn.h"

InterfaceTable* ft;

void registerSDMaps();
void registerSDFlows();
void registerSDProcesses();
void registerSDDistributions();
void registerSDStateProcesses();
void registerSDMapSonifiers();
void registerSDAdvancedEvents();
void registerSDShotNoise();

PluginLoad(Stochastica)
{
    ft = inTable;
    registerSDMaps();
    registerSDFlows();
    registerSDProcesses();
    registerSDDistributions();
    registerSDStateProcesses();
    registerSDMapSonifiers();
    registerSDAdvancedEvents();
    registerSDShotNoise();
}

