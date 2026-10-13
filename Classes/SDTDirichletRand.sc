TDirichletRand : MultiOutUGen {
    *ar { |numChannels=4, alphaBuf=0, trig=1, reload=0, seed=0, reset=0|
        ^this.multiNew('audio', numChannels.asInteger.clip(2,32), alphaBuf, trig, reload, seed, reset)
    }
    *kr { |numChannels=4, alphaBuf=0, trig=1, reload=0, seed=0, reset=0|
        ^this.multiNew('control', numChannels.asInteger.clip(2,32), alphaBuf, trig, reload, seed, reset)
    }
    init { |numChannels ... inputs| this.inputs=inputs; ^this.initOutputs(numChannels,rate) }
    argNamesInputsOffset { ^2 }
}
