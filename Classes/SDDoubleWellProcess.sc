DoubleWellProcess : UGen {
    *ar { |drive = 0, a = 1, b = 1, noise = 0.35, timeScale = 1, initial = -1, substeps = 1, seed = 0, reset = 0|
        ^this.multiNew('audio', drive, a, b, noise, timeScale, initial, substeps.asInteger.clip(1, 16), seed, reset)
    }
    *kr { |drive = 0, a = 1, b = 1, noise = 0.35, timeScale = 1, initial = -1, substeps = 1, seed = 0, reset = 0|
        ^this.multiNew('control', drive, a, b, noise, timeScale, initial, substeps.asInteger.clip(1, 16), seed, reset)
    }
}
