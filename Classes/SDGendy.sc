Gendy1977 : UGen {
    *ar { |ampDist=0, durDist=0, ampShape=1, durShape=1, minFreq=100, maxFreq=1000, ampStep=0.1, durStep=0.1, numPoints=12, reset=0, seed=0|
        ^this.multiNew('audio', ampDist, durDist, ampShape, durShape, minFreq, maxFreq, ampStep, durStep, numPoints, reset, seed)
    }
}
Gendy1991 : UGen {
    *ar { |ampDist=0, durDist=0, ampShape=1, durShape=1, minFreq=100, maxFreq=1000, ampPrimaryRange=0.1, durPrimaryRatio=0.5, ampPrimaryStep=0.1, durPrimaryStep=0.1, numPoints=12, reset=0, seed=0|
        ^this.multiNew('audio', ampDist, durDist, ampShape, durShape, minFreq, maxFreq, ampPrimaryRange, durPrimaryRatio, ampPrimaryStep, durPrimaryStep, numPoints, reset, seed)
    }
}
GendyS709 : UGen {
    *ar { |ampDist=0, durDist=0, minFreq=100, maxFreq=1000, ampStep=0.1, durStep=0.1, modFreq=0.2, modShape=0, ampStepDepth=0.5, durStepDepth=0.5, freqRangeDepth=0, numPoints=12, reset=0, seed=0|
        ^this.multiNew('audio', ampDist, durDist, minFreq, maxFreq, ampStep, durStep, modFreq, modShape, ampStepDepth, durStepDepth, freqRangeDepth, numPoints, reset, seed)
    }
}
GendyField : MultiOutUGen {
    *kr { |numVoices=4, meanDuration=1, soundProb=0.5, reset=0, seed=0|
        ^this.multiNew('control', numVoices.asInteger.clip(1,32), meanDuration, soundProb, reset, seed)
    }
    init { |numVoices ... inputs|
        this.inputs=inputs;
        ^this.initOutputs(numVoices,rate)
    }
    argNamesInputsOffset { ^2 }
}
GendyPerc : UGen {
    *ar { |trig=0, decay=1, minFreq=100, maxFreq=2000, startAmpStep=0.5, endAmpStep=0.001, startDurStep=0.5, endDurStep=0.001, numPoints=12, resetState=1, seed=0|
        ^this.multiNew('audio', trig, decay, minFreq, maxFreq, startAmpStep, endAmpStep, startDurStep, endDurStep, numPoints, resetState, seed)
    }
}
GendyInterp : UGen {
    *ar { |interp=1, ampDist=0, durDist=0, minFreq=100, maxFreq=1000, ampStep=0.1, durStep=0.1, numPoints=12, reset=0, seed=0|
        ^this.multiNew('audio', interp, ampDist, durDist, minFreq, maxFreq, ampStep, durStep, numPoints, reset, seed)
    }
}
GendyCoupled : UGen {
    *ar { |ampCoupling=0.1, durCoupling=0, ampDist=0, durDist=0, minFreq=100, maxFreq=1000, ampStep=0.1, durStep=0.1, numPoints=12, reset=0, seed=0|
        ^this.multiNew('audio', ampCoupling, durCoupling, ampDist, durDist, minFreq, maxFreq, ampStep, durStep, numPoints, reset, seed)
    }
}
GendyDiffusion : UGen {
    *ar { |freq=220, diffusivity=1, dimensions=2, timeStep=0.001, modFreq=0.2, numPoints=36, reset=0, seed=0|
        ^this.multiNew('audio', freq, diffusivity, dimensions, timeStep, modFreq, numPoints, reset, seed)
    }
}
