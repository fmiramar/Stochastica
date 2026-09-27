HenonSonify : UGen {
	*ar { |minFreq = 20, maxFreq = 2000, clockLo = -1.5, clockHi = 1.5,
		outputLo = -0.5, outputHi = 0.5, clockIndex = 0, outputIndex = 1,
		interp = 2, a = 1.4, b = 0.3, x0 = 0, y0 = 0, reset = 0,
		mul = 1, add = 0|
		^this.multiNew('audio', minFreq, maxFreq, clockLo, clockHi,
			outputLo, outputHi, clockIndex, outputIndex, interp,
			a, b, x0, y0, reset).madd(mul, add)
	}
}

GbmanSonify : UGen {
	*ar { |minFreq = 20, maxFreq = 2000, clockLo = -10, clockHi = 10,
		outputLo = -10, outputHi = 10, clockIndex = 0, outputIndex = 1,
		interp = 2, x0 = 1.2, y0 = 2.1, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', minFreq, maxFreq, clockLo, clockHi,
			outputLo, outputHi, clockIndex, outputIndex, interp,
			x0, y0, reset).madd(mul, add)
	}
}

LatoocarfianSonify : UGen {
	*ar { |minFreq = 20, maxFreq = 2000, clockLo = -1.5, clockHi = 1.5,
		outputLo = -1.5, outputHi = 1.5, clockIndex = 0, outputIndex = 1,
		interp = 2, a = 1, b = 3, c = 0.5, d = 0.5,
		x0 = 0.1, y0 = 0.1, reset = 0, mul = 1, add = 0|
		^this.multiNew('audio', minFreq, maxFreq, clockLo, clockHi,
			outputLo, outputHi, clockIndex, outputIndex, interp,
			a, b, c, d, x0, y0, reset).madd(mul, add)
	}
}

