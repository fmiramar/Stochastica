#pragma once
#include "SDRound3Harness.hpp"
void registerSDGendy();
namespace sdtest {
inline const std::vector<Spec> gendySpecs={
    {"Gendy77",{0,0,1,1,100,1000,.1f,.1f,12,0,123},1,10,9,-1,-1},
    {"Gendy91",{0,0,1,1,100,1000,.1f,.5f,.1f,.1f,12,0,123},1,12,11,-1,-1},
    {"GendyS709",{0,0,100,1000,.1f,.1f,.2f,0,.5f,.5f,0,12,0,123},1,13,12,-1,-1},
    {"GendyPerc",{0,1,100,2000,.5f,.001f,.5f,.001f,12,1,123},1,10,0,-1,-1},
    {"GendyInterp",{1,0,0,100,1000,.1f,.1f,12,0,123},1,9,8,-1,-1},
    {"GendyCoupled",{.1f,0,0,0,100,1000,.1f,.1f,12,0,123},1,10,9,-1,-1},
    {"GendyField",{1,.5f,0,123},4,3,2,-1,-1},
    {"GendyDiffusion",{220,1,2,.001f,.2f,36,0,123},1,7,6,-1,-1}
};
inline void initializeGendy(InterfaceTable& table) { ft=&table;table.fDefineUnit=define;table.fRTAlloc=allocate;table.fRTFree=release;table.fClearUnitOutputs=clear;registerSDGendy(); }
}
