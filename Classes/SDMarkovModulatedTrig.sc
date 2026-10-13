MarkovModulatedTrig : MultiOutUGen {
    *ar { |numStates=3, transitionBuf=0, eventRateBuf=1, stateRateBuf=2, initialState=0, maxEventRate=20000, maxStateRate=20000, reload=0, seed=0, reset=0|
        ^this.multiNew('audio',numStates.asInteger.clip(2,64),transitionBuf,eventRateBuf,stateRateBuf,initialState,maxEventRate,maxStateRate,reload,seed,reset)
    }
    *kr { |numStates=3, transitionBuf=0, eventRateBuf=1, stateRateBuf=2, initialState=0, maxEventRate=20000, maxStateRate=20000, reload=0, seed=0, reset=0|
        ^this.multiNew('control',numStates.asInteger.clip(2,64),transitionBuf,eventRateBuf,stateRateBuf,initialState,maxEventRate,maxStateRate,reload,seed,reset)
    }
    init { |... inputs| this.inputs=inputs; ^this.initOutputs(4,rate) }
}
