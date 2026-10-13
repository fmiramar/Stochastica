MultiHawkesTrig : MultiOutUGen {
    *ar { |numProcesses = 2, baseRateBuf = 0, excitationBuf = 1, decayBuf = 2, maxRate = 20000, reload = 0, seed = 0, reset = 0|
        ^this.multiNew('audio', numProcesses.asInteger.clip(1, 16), baseRateBuf, excitationBuf, decayBuf, maxRate, reload, seed, reset)
    }
    *kr { |numProcesses = 2, baseRateBuf = 0, excitationBuf = 1, decayBuf = 2, maxRate = 20000, reload = 0, seed = 0, reset = 0|
        ^this.multiNew('control', numProcesses.asInteger.clip(1, 16), baseRateBuf, excitationBuf, decayBuf, maxRate, reload, seed, reset)
    }
    init { |numProcesses ... inputs|
        this.inputs = inputs;
        ^this.initOutputs(2 * numProcesses, rate)
    }
    argNamesInputsOffset { ^2 }
}
