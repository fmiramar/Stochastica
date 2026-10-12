SelfCorrectingTrig : MultiOutUGen {
    *ar { |rate = 1, correction = 1, maxRate = 20000, seed = 0, reset = 0|
        ^this.multiNew('audio', rate, correction, maxRate, seed, reset)
    }
    *kr { |rate = 1, correction = 1, maxRate = 20000, seed = 0, reset = 0|
        ^this.multiNew('control', rate, correction, maxRate, seed, reset)
    }
    init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

RandomTelegraph : MultiOutUGen {
    *ar { |low = -1, high = 1, upRate = 1, downRate = 1, initialState = 0, seed = 0, reset = 0|
        ^this.multiNew('audio', low, high, upRate, downRate, initialState, seed, reset)
    }
    *kr { |low = -1, high = 1, upRate = 1, downRate = 1, initialState = 0, seed = 0, reset = 0|
        ^this.multiNew('control', low, high, upRate, downRate, initialState, seed, reset)
    }
    init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

ShotNoise : MultiOutUGen {
    *ar { |rate = 10, decay = 0.05, ampMean = 0, ampStd = 1, seed = 0, reset = 0|
        ^this.multiNew('audio', rate, decay, ampMean, ampStd, seed, reset)
    }
    *kr { |rate = 10, decay = 0.05, ampMean = 0, ampStd = 1, seed = 0, reset = 0|
        ^this.multiNew('control', rate, decay, ampMean, ampStd, seed, reset)
    }
    init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}
