MarkovChain : MultiOutUGen {
	*ar { |numStates = 2, trig = 0, transitionBuf = 0, valueBuf = -1, initialState = 0, seed = 0, reset = 0|
		numStates = numStates.asInteger.clip(1, 256);
		^this.multiNew('audio', numStates, trig, transitionBuf, valueBuf, initialState, seed, reset)
	}
	*kr { |numStates = 2, trig = 0, transitionBuf = 0, valueBuf = -1, initialState = 0, seed = 0, reset = 0|
		numStates = numStates.asInteger.clip(1, 256);
		^this.multiNew('control', numStates, trig, transitionBuf, valueBuf, initialState, seed, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(2, rate) }
}

SemiMarkov : MultiOutUGen {
	*ar { |numStates = 2, transitionBuf = 0, durationBuf = 0, valueBuf = -1, initialState = 0, timeScale = 1, seed = 0, reset = 0|
		numStates = numStates.asInteger.clip(1, 256);
		^this.multiNew('audio', numStates, transitionBuf, durationBuf, valueBuf, initialState, timeScale, seed, reset)
	}
	*kr { |numStates = 2, transitionBuf = 0, durationBuf = 0, valueBuf = -1, initialState = 0, timeScale = 1, seed = 0, reset = 0|
		numStates = numStates.asInteger.clip(1, 256);
		^this.multiNew('control', numStates, transitionBuf, durationBuf, valueBuf, initialState, timeScale, seed, reset)
	}
	init { |... inputs| this.inputs = inputs; ^this.initOutputs(4, rate) }
}

MultiGaussNoise : MultiOutUGen {
	*ar { |numChannels = 2, trig = 0, covarianceBuf = 0, meanBuf = -1, reload = 0, seed = 0, reset = 0|
		numChannels = numChannels.asInteger.clip(1, 32);
		^this.multiNew('audio', numChannels, trig, covarianceBuf, meanBuf, reload, seed, reset)
	}
	*kr { |numChannels = 2, trig = 0, covarianceBuf = 0, meanBuf = -1, reload = 0, seed = 0, reset = 0|
		numChannels = numChannels.asInteger.clip(1, 32);
		^this.multiNew('control', numChannels, trig, covarianceBuf, meanBuf, reload, seed, reset)
	}
	init { |numChannels ... inputs|
		this.inputs = inputs;
		^this.initOutputs(numChannels, rate)
	}
	argNamesInputsOffset { ^2 }
}

