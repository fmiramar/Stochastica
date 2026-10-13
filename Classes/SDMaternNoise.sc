MaternNoise : UGen {
    *ar { |mean=0, variance=1, correlationTime=1, smoothness=1, timeScale=1, initial=0, seed=0, reset=0|
        ^this.multiNew('audio',mean,variance,correlationTime,smoothness,timeScale,initial,seed,reset)
    }
    *kr { |mean=0, variance=1, correlationTime=1, smoothness=1, timeScale=1, initial=0, seed=0, reset=0|
        ^this.multiNew('control',mean,variance,correlationTime,smoothness,timeScale,initial,seed,reset)
    }
}
