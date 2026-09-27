#include "SC_PlugIn.h"

InterfaceTable* ft;

void registerSDMaps();
void registerSDFlows();
void registerSDProcesses();
void registerSDDistributions();
void registerSDStateProcesses();
void registerSDMapSonifiers();

PluginLoad(Stochastica)
{
    ft = inTable;
    registerSDMaps();
    registerSDFlows();
    registerSDProcesses();
    registerSDDistributions();
    registerSDStateProcesses();
    registerSDMapSonifiers();
}

