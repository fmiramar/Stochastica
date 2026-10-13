JumpDiffusion : MultiOutUGen {
    *ar { |drift=0, diffusion=1, jumpRate=0.5, jumpMean=0, jumpStd=3, timeScale=1, initial=0, seed=0, reset=0|
        ^this.multiNew('audio',drift,diffusion,jumpRate,jumpMean,jumpStd,timeScale,initial,seed,reset)
    }
    *kr { |drift=0, diffusion=1, jumpRate=0.5, jumpMean=0, jumpStd=3, timeScale=1, initial=0, seed=0, reset=0|
        ^this.multiNew('control',drift,diffusion,jumpRate,jumpMean,jumpStd,timeScale,initial,seed,reset)
    }
    init { |... inputs| this.inputs=inputs; ^this.initOutputs(4,rate) }
}
