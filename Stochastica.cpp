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
void registerSDCircularDistributions();
void registerSDMultiOU();
void registerSDMultiHawkes();
void registerSDDoubleWell();
void registerSDKuramoto();

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
    registerSDCircularDistributions();
    registerSDMultiOU();
    registerSDMultiHawkes();
    registerSDDoubleWell();
    registerSDKuramoto();
}

