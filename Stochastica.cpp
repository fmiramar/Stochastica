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
void registerSDDirichlet();
void registerSDPolya();
void registerSDModulated();
void registerSDMatern();
void registerSDLevyFlight();
void registerSDJumpDiffusion();
void registerSDWilsonCowan();
void registerSDLorenz96();
void registerSDGendy();

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
    registerSDDirichlet();
    registerSDPolya();
    registerSDModulated();
    registerSDMatern();
    registerSDLevyFlight();
    registerSDJumpDiffusion();
    registerSDWilsonCowan();
    registerSDLorenz96();
    registerSDGendy();
}

