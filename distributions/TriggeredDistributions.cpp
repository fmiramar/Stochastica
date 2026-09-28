#include "SC_PlugIn.h"
#include "SDDistributions.hpp"
#include "SDServer.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

extern InterfaceTable* ft;

namespace {

struct SamplerUnit : Unit {
    sd::PCG32 rng;
    uint32_t originalSeed = 1U;
    double value = 0.0;
    sd::RisingEdge trigger;
    sd::RisingEdge reset;
};

struct TGammaRand : SamplerUnit {};
struct TWeibullRand : SamplerUnit {};
struct TLogNormalRand : SamplerUnit {};
struct TCauchyRand : SamplerUnit {};
struct TPoissonRand : SamplerUnit {};
struct TGeometricRand : SamplerUnit {};
struct TNegBinomialRand : SamplerUnit {};
struct TTruncNormalRand : SamplerUnit {};
struct TStableRand : SamplerUnit {};
struct TLevyRand : SamplerUnit {};

template <typename UnitType, typename Sample>
void processSampler(UnitType* unit, int inNumSamples, int triggerIndex,
                    int seedIndex, int resetIndex, Sample sampleValue)
{
    float* output = OUT(0);
    for (int sample = 0; sample < inNumSamples; ++sample) {
        const bool resetEdge = unit->reset.update(sd::inputAt(unit, resetIndex, sample));
        const bool triggerEdge = unit->trigger.update(sd::inputAt(unit, triggerIndex, sample));
        if (resetEdge) {
            unit->rng.setSeed(unit->originalSeed);
            unit->value = sampleValue(sample);
        } else if (triggerEdge) {
            unit->value = sampleValue(sample);
        }
        if (!sd::finite(unit->value))
            unit->value = unit->value < 0.0
                ? -static_cast<double>(std::numeric_limits<float>::max())
                : static_cast<double>(std::numeric_limits<float>::max());
        output[sample] = sd::finiteFloat(unit->value);
    }
    (void)seedIndex;
}

template <typename UnitType, typename Sample>
void initializeSampler(UnitType* unit, int triggerIndex, int seedIndex,
                       int resetIndex, Sample sampleValue)
{
    unit->originalSeed = sd::seedForUnit(unit, seedIndex);
    unit->rng.setSeed(unit->originalSeed);
    unit->trigger.initialize(IN0(triggerIndex));
    unit->reset.initialize(IN0(resetIndex));
    unit->value = sampleValue(0);
}

double gammaValue(TGammaRand* unit, int sample)
{
    const double shape = sd::inputAt(unit, 0, sample);
    const double scale = sd::inputAt(unit, 1, sample);
    if (!(shape > 0.0) || !(scale >= 0.0) || !sd::finite(shape) || !sd::finite(scale))
        return sd::finite(shape * scale) && shape * scale >= 0.0 ? shape * scale : 0.0;
    return sd::gammaSample(unit->rng, shape, scale);
}

void TGammaRand_next(TGammaRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 2, 3, 4,
                   [unit](int sample) { return gammaValue(unit, sample); });
}

void TGammaRand_Ctor(TGammaRand* unit)
{
    initializeSampler(unit, 2, 3, 4, [unit](int sample) { return gammaValue(unit, sample); });
    SETCALC(TGammaRand_next);
    TGammaRand_next(unit, 1);
}

double weibullValue(TWeibullRand* unit, int sample)
{
    return sd::weibullSample(
        unit->rng, sd::inputAt(unit, 0, sample), sd::inputAt(unit, 1, sample));
}

void TWeibullRand_next(TWeibullRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 2, 3, 4,
                   [unit](int sample) { return weibullValue(unit, sample); });
}

void TWeibullRand_Ctor(TWeibullRand* unit)
{
    initializeSampler(unit, 2, 3, 4, [unit](int sample) { return weibullValue(unit, sample); });
    SETCALC(TWeibullRand_next);
    TWeibullRand_next(unit, 1);
}

double logNormalValue(TLogNormalRand* unit, int sample)
{
    return sd::logNormalSample(
        unit->rng, sd::inputAt(unit, 0, sample), sd::inputAt(unit, 1, sample));
}

void TLogNormalRand_next(TLogNormalRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 2, 3, 4,
                   [unit](int sample) { return logNormalValue(unit, sample); });
}

void TLogNormalRand_Ctor(TLogNormalRand* unit)
{
    initializeSampler(unit, 2, 3, 4, [unit](int sample) { return logNormalValue(unit, sample); });
    SETCALC(TLogNormalRand_next);
    TLogNormalRand_next(unit, 1);
}

double cauchyValue(TCauchyRand* unit, int sample)
{
    return sd::cauchySample(
        unit->rng, sd::inputAt(unit, 0, sample), sd::inputAt(unit, 1, sample));
}

void TCauchyRand_next(TCauchyRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 2, 3, 4,
                   [unit](int sample) { return cauchyValue(unit, sample); });
}

void TCauchyRand_Ctor(TCauchyRand* unit)
{
    initializeSampler(unit, 2, 3, 4, [unit](int sample) { return cauchyValue(unit, sample); });
    SETCALC(TCauchyRand_next);
    TCauchyRand_next(unit, 1);
}

double poissonValue(TPoissonRand* unit, int sample)
{
    const double lambda = sd::inputAt(unit, 0, sample);
    return static_cast<double>(sd::poissonSample(unit->rng, std::max(sd::finiteOr(lambda, 0.0), 0.0)));
}

void TPoissonRand_next(TPoissonRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 1, 2, 3,
                   [unit](int sample) { return poissonValue(unit, sample); });
}

void TPoissonRand_Ctor(TPoissonRand* unit)
{
    initializeSampler(unit, 1, 2, 3, [unit](int sample) { return poissonValue(unit, sample); });
    SETCALC(TPoissonRand_next);
    TPoissonRand_next(unit, 1);
}

double geometricValue(TGeometricRand* unit, int sample)
{
    return sd::geometricSample(unit->rng, sd::inputAt(unit, 0, sample));
}

void TGeometricRand_next(TGeometricRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 1, 2, 3,
                   [unit](int sample) { return geometricValue(unit, sample); });
}

void TGeometricRand_Ctor(TGeometricRand* unit)
{
    initializeSampler(unit, 1, 2, 3, [unit](int sample) { return geometricValue(unit, sample); });
    SETCALC(TGeometricRand_next);
    TGeometricRand_next(unit, 1);
}

double negativeBinomialValue(TNegBinomialRand* unit, int sample)
{
    const double successes = sd::inputAt(unit, 0, sample);
    const double probability = sd::inputAt(unit, 1, sample);
    if (!(successes > 0.0) || !sd::finite(successes))
        return 0.0;
    if (!(probability > 0.0))
        return sd::kLargestExactIntegerFloat;
    if (probability >= 1.0)
        return 0.0;
    const double lambda = sd::gammaSample(
        unit->rng, successes, (1.0 - probability) / probability);
    return static_cast<double>(sd::poissonSample(unit->rng, lambda));
}

void TNegBinomialRand_next(TNegBinomialRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 2, 3, 4,
                   [unit](int sample) { return negativeBinomialValue(unit, sample); });
}

void TNegBinomialRand_Ctor(TNegBinomialRand* unit)
{
    initializeSampler(unit, 2, 3, 4,
                      [unit](int sample) { return negativeBinomialValue(unit, sample); });
    SETCALC(TNegBinomialRand_next);
    TNegBinomialRand_next(unit, 1);
}

double truncatedNormalValue(TTruncNormalRand* unit, int sample)
{
    return sd::truncatedNormalSample(
        unit->rng, sd::inputAt(unit, 0, sample), sd::inputAt(unit, 1, sample),
        sd::inputAt(unit, 2, sample), sd::inputAt(unit, 3, sample));
}

void TTruncNormalRand_next(TTruncNormalRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 4, 5, 6,
                   [unit](int sample) { return truncatedNormalValue(unit, sample); });
}

void TTruncNormalRand_Ctor(TTruncNormalRand* unit)
{
    initializeSampler(unit, 4, 5, 6,
                      [unit](int sample) { return truncatedNormalValue(unit, sample); });
    SETCALC(TTruncNormalRand_next);
    TTruncNormalRand_next(unit, 1);
}

double stableValue(TStableRand* unit, int sample)
{
    return sd::stableS0Sample(
        unit->rng, sd::inputAt(unit, 0, sample), sd::inputAt(unit, 1, sample),
        sd::inputAt(unit, 2, sample), sd::inputAt(unit, 3, sample));
}

void TStableRand_next(TStableRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 4, 5, 6,
                   [unit](int sample) { return stableValue(unit, sample); });
}

void TStableRand_Ctor(TStableRand* unit)
{
    initializeSampler(unit, 4, 5, 6, [unit](int sample) { return stableValue(unit, sample); });
    SETCALC(TStableRand_next);
    TStableRand_next(unit, 1);
}

double levyValue(TLevyRand* unit, int sample)
{
    const double location = sd::finiteOr(sd::inputAt(unit, 0, sample), 0.0);
    const double scale = sd::inputAt(unit, 1, sample);
    if (!(scale >= 0.0) || !sd::finite(scale))
        return location;
    return sd::levySample(unit->rng, location, scale);
}

void TLevyRand_next(TLevyRand* unit, int inNumSamples)
{
    processSampler(unit, inNumSamples, 2, 3, 4,
                   [unit](int sample) { return levyValue(unit, sample); });
}

void TLevyRand_Ctor(TLevyRand* unit)
{
    initializeSampler(unit, 2, 3, 4, [unit](int sample) { return levyValue(unit, sample); });
    SETCALC(TLevyRand_next);
    TLevyRand_next(unit, 1);
}

} // namespace

void registerSDDistributions()
{
    DefineSimpleUnit(TGammaRand);
    DefineSimpleUnit(TWeibullRand);
    DefineSimpleUnit(TLogNormalRand);
    DefineSimpleUnit(TCauchyRand);
    DefineSimpleUnit(TPoissonRand);
    DefineSimpleUnit(TGeometricRand);
    DefineSimpleUnit(TNegBinomialRand);
    DefineSimpleUnit(TTruncNormalRand);
    DefineSimpleUnit(TStableRand);
    DefineSimpleUnit(TLevyRand);
}

