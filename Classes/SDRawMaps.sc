HenonMap : MultiOutUGen {
	*ar { |trig = 0, a = 1.4, b = 0.3, x0 = 0, y0 = 0, reset = 0|
		^this.multiNew('audio', trig, a, b, x0, y0, reset)
	}
	*kr { |trig = 0, a = 1.4, b = 0.3, x0 = 0, y0 = 0, reset = 0|
		^this.multiNew('control', trig, a, b, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

GbmanMap : MultiOutUGen {
	*ar { |trig = 0, x0 = 1.2, y0 = 2.1, reset = 0|
		^this.multiNew('audio', trig, x0, y0, reset)
	}
	*kr { |trig = 0, x0 = 1.2, y0 = 2.1, reset = 0|
		^this.multiNew('control', trig, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

LatoocarfianMap : MultiOutUGen {
	*ar { |trig = 0, a = 1, b = 3, c = 0.5, d = 0.5, x0 = 0.1, y0 = 0.1, reset = 0|
		^this.multiNew('audio', trig, a, b, c, d, x0, y0, reset)
	}
	*kr { |trig = 0, a = 1, b = 3, c = 0.5, d = 0.5, x0 = 0.1, y0 = 0.1, reset = 0|
		^this.multiNew('control', trig, a, b, c, d, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

IkedaMap : MultiOutUGen {
	*ar { |trig = 0, u = 0.9, x0 = 0.1, y0 = 0.1, reset = 0|
		^this.multiNew('audio', trig, u, x0, y0, reset)
	}
	*kr { |trig = 0, u = 0.9, x0 = 0.1, y0 = 0.1, reset = 0|
		^this.multiNew('control', trig, u, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

LoziMap : MultiOutUGen {
	*ar { |trig = 0, a = 1.7, b = 0.5, x0 = 0.1, y0 = 0, reset = 0|
		^this.multiNew('audio', trig, a, b, x0, y0, reset)
	}
	*kr { |trig = 0, a = 1.7, b = 0.5, x0 = 0.1, y0 = 0, reset = 0|
		^this.multiNew('control', trig, a, b, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

TinkerbellMap : MultiOutUGen {
	*ar { |trig = 0, a = 0.9, b = -0.6013, c = 2, d = 0.5, x0 = -0.72, y0 = -0.64, reset = 0|
		^this.multiNew('audio', trig, a, b, c, d, x0, y0, reset)
	}
	*kr { |trig = 0, a = 0.9, b = -0.6013, c = 2, d = 0.5, x0 = -0.72, y0 = -0.64, reset = 0|
		^this.multiNew('control', trig, a, b, c, d, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

CliffordMap : MultiOutUGen {
	*ar { |trig = 0, a = -1.4, b = 1.6, c = 1, d = 0.7, x0 = 0.1, y0 = 0.1, reset = 0|
		^this.multiNew('audio', trig, a, b, c, d, x0, y0, reset)
	}
	*kr { |trig = 0, a = -1.4, b = 1.6, c = 1, d = 0.7, x0 = 0.1, y0 = 0.1, reset = 0|
		^this.multiNew('control', trig, a, b, c, d, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

DeJongMap : MultiOutUGen {
	*ar { |trig = 0, a = 1.4, b = -2.3, c = 2.4, d = -2.1, x0 = 0, y0 = 0, reset = 0|
		^this.multiNew('audio', trig, a, b, c, d, x0, y0, reset)
	}
	*kr { |trig = 0, a = 1.4, b = -2.3, c = 2.4, d = -2.1, x0 = 0, y0 = 0, reset = 0|
		^this.multiNew('control', trig, a, b, c, d, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

CircleMap : UGen {
	*ar { |trig = 0, omega = 0.2, k = 0.9, x0 = 0, reset = 0|
		^this.multiNew('audio', trig, omega, k, x0, reset)
	}
	*kr { |trig = 0, omega = 0.2, k = 0.9, x0 = 0, reset = 0|
		^this.multiNew('control', trig, omega, k, x0, reset)
	}
}

CoupledLogisticMap : MultiOutUGen {
	*ar { |trig = 0, rX = 3.8, rY = 3.8, coupling = 0.05, x0 = 0.2, y0 = 0.21, reset = 0|
		^this.multiNew('audio', trig, rX, rY, coupling, x0, y0, reset)
	}
	*kr { |trig = 0, rX = 3.8, rY = 3.8, coupling = 0.05, x0 = 0.2, y0 = 0.21, reset = 0|
		^this.multiNew('control', trig, rX, rY, coupling, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

CoupledMapLattice : MultiOutUGen {
	*ar { |numCells = 8, trig = 0, r = 3.8, coupling = 0.1, initialBuf = 0, reset = 0|
		numCells = numCells.asInteger.clip(2, 64);
		^this.multiNew('audio', numCells, trig, r, coupling, initialBuf, reset)
	}
	*kr { |numCells = 8, trig = 0, r = 3.8, coupling = 0.1, initialBuf = 0, reset = 0|
		numCells = numCells.asInteger.clip(2, 64);
		^this.multiNew('control', numCells, trig, r, coupling, initialBuf, reset)
	}
	init { |numCells ... inputs|
		this.inputs = inputs;
		^this.initOutputs(numCells, rate)
	}
	argNamesInputsOffset { ^2 }
}

RulkovMap : MultiOutUGen {
	*ar { |trig = 0, alpha = 4.1, mu = 0.001, sigma = -1.6, x0 = -1, y0 = -2.9, reset = 0|
		^this.multiNew('audio', trig, alpha, mu, sigma, x0, y0, reset)
	}
	*kr { |trig = 0, alpha = 4.1, mu = 0.001, sigma = -1.6, x0 = -1, y0 = -2.9, reset = 0|
		^this.multiNew('control', trig, alpha, mu, sigma, x0, y0, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

