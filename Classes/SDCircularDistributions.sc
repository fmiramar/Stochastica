TVonMisesRand : UGen {
    *ar { |mean = 0, kappa = 1, trig = 0, seed = 0, reset = 0|
        ^this.multiNew('audio', mean, kappa, trig, seed, reset)
    }
    *kr { |mean = 0, kappa = 1, trig = 0, seed = 0, reset = 0|
        ^this.multiNew('control', mean, kappa, trig, seed, reset)
    }
}
