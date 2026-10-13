LevyFlight : MultiOutUGen {
    *ar { |alpha=1.5, scale=1, drift=0, timeScale=1, initial=0, seed=0, reset=0|
        ^this.multiNew('audio',alpha,scale,drift,timeScale,initial,seed,reset)
    }
    *kr { |alpha=1.5, scale=1, drift=0, timeScale=1, initial=0, seed=0, reset=0|
        ^this.multiNew('control',alpha,scale,drift,timeScale,initial,seed,reset)
    }
    init { |... inputs| this.inputs=inputs; ^this.initOutputs(2,rate) }
}
