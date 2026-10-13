MultiOUProcess : MultiOutUGen {
    *ar { |numChannels = 2, covarianceBuf = 0, meanBuf = -1, reversionBuf = -1, reload = 0, seed = 0, reset = 0|
        ^this.multiNew('audio', numChannels.asInteger.clip(1, 32), covarianceBuf, meanBuf, reversionBuf, reload, seed, reset)
    }
    *kr { |numChannels = 2, covarianceBuf = 0, meanBuf = -1, reversionBuf = -1, reload = 0, seed = 0, reset = 0|
        ^this.multiNew('control', numChannels.asInteger.clip(1, 32), covarianceBuf, meanBuf, reversionBuf, reload, seed, reset)
    }
    init { |numChannels ... inputs|
        this.inputs = inputs;
        ^this.initOutputs(numChannels, rate)
    }
    argNamesInputsOffset { ^2 }
}
