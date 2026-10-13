PolyaUrn : MultiOutUGen {
    *ar { |numStates=4, weightBuf=0, trig=1, reinforcement=1, valueBuf = -1, reload=0, seed=0, reset=0|
        ^this.multiNew('audio', numStates.asInteger.clip(2,64), weightBuf, trig, reinforcement, valueBuf, reload, seed, reset)
    }
    *kr { |numStates=4, weightBuf=0, trig=1, reinforcement=1, valueBuf = -1, reload=0, seed=0, reset=0|
        ^this.multiNew('control', numStates.asInteger.clip(2,64), weightBuf, trig, reinforcement, valueBuf, reload, seed, reset)
    }
    init { |... inputs| this.inputs=inputs; ^this.initOutputs(4,rate) }
}
