WilsonCowan : MultiOutUGen {
    *ar { |inputExc=0, inputInh=0, wEE=10, wEI=12, wIE=10, wII=2, tauExc=0.02, tauInh=0.01, gainExc=1, thresholdExc=0, gainInh=1, thresholdInh=0, timeScale=1, substeps=1, initialExc=0.1, initialInh=0.1, reset=0|
        ^this.multiNew('audio',inputExc,inputInh,wEE,wEI,wIE,wII,tauExc,tauInh,gainExc,thresholdExc,gainInh,thresholdInh,timeScale,substeps,initialExc,initialInh,reset)
    }
    *kr { |inputExc=0, inputInh=0, wEE=10, wEI=12, wIE=10, wII=2, tauExc=0.02, tauInh=0.01, gainExc=1, thresholdExc=0, gainInh=1, thresholdInh=0, timeScale=1, substeps=1, initialExc=0.1, initialInh=0.1, reset=0|
        ^this.multiNew('control',inputExc,inputInh,wEE,wEI,wIE,wII,tauExc,tauInh,gainExc,thresholdExc,gainInh,thresholdInh,timeScale,substeps,initialExc,initialInh,reset)
    }
    init { |... inputs| this.inputs=inputs; ^this.initOutputs(2,rate) }
}
