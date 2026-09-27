TGammaRand : UGen {
	*ar { |shape = 2, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', shape, scale, trig, seed, reset).madd(mul, add)
	}
	*kr { |shape = 2, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', shape, scale, trig, seed, reset).madd(mul, add)
	}
}

TWeibullRand : UGen {
	*ar { |shape = 1, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', shape, scale, trig, seed, reset).madd(mul, add)
	}
	*kr { |shape = 1, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', shape, scale, trig, seed, reset).madd(mul, add)
	}
}

TLogNormalRand : UGen {
	*ar { |mu = 0, sigma = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', mu, sigma, trig, seed, reset).madd(mul, add)
	}
	*kr { |mu = 0, sigma = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', mu, sigma, trig, seed, reset).madd(mul, add)
	}
}

TCauchyRand : UGen {
	*ar { |location = 0, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', location, scale, trig, seed, reset).madd(mul, add)
	}
	*kr { |location = 0, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', location, scale, trig, seed, reset).madd(mul, add)
	}
}

TPoissonRand : UGen {
	*ar { |lambda = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', lambda, trig, seed, reset).madd(mul, add)
	}
	*kr { |lambda = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', lambda, trig, seed, reset).madd(mul, add)
	}
}

TGeometricRand : UGen {
	*ar { |probability = 0.5, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', probability, trig, seed, reset).madd(mul, add)
	}
	*kr { |probability = 0.5, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', probability, trig, seed, reset).madd(mul, add)
	}
}

TNegBinomialRand : UGen {
	*ar { |successes = 2, probability = 0.5, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', successes, probability, trig, seed, reset).madd(mul, add)
	}
	*kr { |successes = 2, probability = 0.5, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', successes, probability, trig, seed, reset).madd(mul, add)
	}
}

TTruncNormalRand : UGen {
	*ar { |mean = 0, deviation = 1, lo = -1, hi = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', mean, deviation, lo, hi, trig, seed, reset).madd(mul, add)
	}
	*kr { |mean = 0, deviation = 1, lo = -1, hi = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', mean, deviation, lo, hi, trig, seed, reset).madd(mul, add)
	}
}

TStableRand : UGen {
	*ar { |alpha = 2, beta = 0, scale = 1, location = 0, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', alpha, beta, scale, location, trig, seed, reset).madd(mul, add)
	}
	*kr { |alpha = 2, beta = 0, scale = 1, location = 0, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', alpha, beta, scale, location, trig, seed, reset).madd(mul, add)
	}
}

TLevyRand : UGen {
	*ar { |location = 0, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', location, scale, trig, seed, reset).madd(mul, add)
	}
	*kr { |location = 0, scale = 1, trig = 0, seed = 0, reset = 0, mul = 1, add = 0|
		^this.multiNew('control', location, scale, trig, seed, reset).madd(mul, add)
	}
}

