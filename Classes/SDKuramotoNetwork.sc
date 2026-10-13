KuramotoNetwork : MultiOutUGen {
    *ar { |numOscillators = 8, frequencyBuf = 0, phaseBuf = -1, coupling = 0, noise = 0, timeScale = 1, substeps = 1, reload = 0, seed = 0, reset = 0|
        ^this.multiNew('audio', numOscillators.asInteger.clip(1, 64), frequencyBuf, phaseBuf, coupling, noise, timeScale, substeps.asInteger.clip(1, 16), reload, seed, reset)
    }
    *kr { |numOscillators = 8, frequencyBuf = 0, phaseBuf = -1, coupling = 0, noise = 0, timeScale = 1, substeps = 1, reload = 0, seed = 0, reset = 0|
        ^this.multiNew('control', numOscillators.asInteger.clip(1, 64), frequencyBuf, phaseBuf, coupling, noise, timeScale, substeps.asInteger.clip(1, 16), reload, seed, reset)
    }
    init { |numOscillators ... inputs|
        this.inputs = inputs;
        ^this.initOutputs(numOscillators + 2, rate)
    }
    argNamesInputsOffset { ^2 }
}
