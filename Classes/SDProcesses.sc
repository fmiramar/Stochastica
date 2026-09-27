OUProcess : UGen {
	*ar { |mean = 0, reversion = 1, diffusion = 1, timeScale = 1, initial = 0, seed = 0, reset = 0|
		^this.multiNew('audio', mean, reversion, diffusion, timeScale, initial, seed, reset)
	}
	*kr { |mean = 0, reversion = 1, diffusion = 1, timeScale = 1, initial = 0, seed = 0, reset = 0|
		^this.multiNew('control', mean, reversion, diffusion, timeScale, initial, seed, reset)
	}
}

RenewalTrig : MultiOutUGen {
	*ar { |rate = 1, distribution = 0, shape = 1, minInterval = 0, seed = 0, reset = 0|
		^this.multiNew('audio', rate, distribution, shape, minInterval, seed, reset)
	}
	*kr { |rate = 1, distribution = 0, shape = 1, minInterval = 0, seed = 0, reset = 0|
		^this.multiNew('control', rate, distribution, shape, minInterval, seed, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

HawkesTrig : MultiOutUGen {
	*ar { |baseRate = 1, excitation = 5, decay = 10, maxRate = 1000, seed = 0, reset = 0|
		^this.multiNew('audio', baseRate, excitation, decay, maxRate, seed, reset)
	}
	*kr { |baseRate = 1, excitation = 5, decay = 10, maxRate = 1000, seed = 0, reset = 0|
		^this.multiNew('control', baseRate, excitation, decay, maxRate, seed, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

FractionalNoise : UGen {
	*ar { |alpha = 1, minFreq = 0.5, maxFreq = 20000, quality = 12, seed = 0, reset = 0|
		^this.multiNew('audio', alpha, minFreq, maxFreq, quality, seed, reset)
	}
	*kr { |alpha = 1, minFreq = 0.01, maxFreq = 20, quality = 12, seed = 0, reset = 0|
		^this.multiNew('control', alpha, minFreq, maxFreq, quality, seed, reset)
	}
}

FBrownianMotion : UGen {
	*kr { |hurst = 0.5, timeScale = 1, initial = 0, memorySize = 256, seed = 0, reset = 0|
		^this.multiNew('control', hurst, timeScale, initial, memorySize, seed, reset)
	}
}

BoundedWalk : UGen {
	*ar { |trig = 0, step = 0.1, lo = -1, hi = 1, boundary = 0, distribution = 0, initial = 0, seed = 0, reset = 0|
		^this.multiNew('audio', trig, step, lo, hi, boundary, distribution, initial, seed, reset)
	}
	*kr { |trig = 0, step = 0.1, lo = -1, hi = 1, boundary = 0, distribution = 0, initial = 0, seed = 0, reset = 0|
		^this.multiNew('control', trig, step, lo, hi, boundary, distribution, initial, seed, reset)
	}
}

