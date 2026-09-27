MackeyGlass : UGen {
	*ar { |beta = 0.2, gamma = 0.1, tau = 17, n = 10, x0 = 1.2, timeScale = 20, maxDelaySeconds = 2, reset = 0|
		^this.multiNew('audio', beta, gamma, tau, n, x0, timeScale, maxDelaySeconds, reset)
	}
	*kr { |beta = 0.2, gamma = 0.1, tau = 17, n = 10, x0 = 1.2, timeScale = 20, maxDelaySeconds = 2, reset = 0|
		^this.multiNew('control', beta, gamma, tau, n, x0, timeScale, maxDelaySeconds, reset)
	}
}

Chua : MultiOutUGen {
	*ar { |alpha = 15.6, beta = 28, m0 = -1.143, m1 = -0.714, x0 = 0.7, y0 = 0, z0 = 0, timeScale = 1, substeps = 2, reset = 0|
		^this.multiNew('audio', alpha, beta, m0, m1, x0, y0, z0, timeScale, substeps, reset)
	}
	*kr { |alpha = 15.6, beta = 28, m0 = -1.143, m1 = -0.714, x0 = 0.7, y0 = 0, z0 = 0, timeScale = 1, substeps = 2, reset = 0|
		^this.multiNew('control', alpha, beta, m0, m1, x0, y0, z0, timeScale, substeps, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(3, rate) }
}

Izhikevich : MultiOutUGen {
	*ar { |input = 10, a = 0.02, b = 0.2, c = -65, d = 8, v0 = -65, u0 = -13, timeScale = 1000, substeps = 2, reset = 0|
		^this.multiNew('audio', input, a, b, c, d, v0, u0, timeScale, substeps, reset)
	}
	*kr { |input = 10, a = 0.02, b = 0.2, c = -65, d = 8, v0 = -65, u0 = -13, timeScale = 1000, substeps = 2, reset = 0|
		^this.multiNew('control', input, a, b, c, d, v0, u0, timeScale, substeps, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(3, rate) }
}

