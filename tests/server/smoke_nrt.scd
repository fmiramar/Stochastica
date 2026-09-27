(
var oscPath = thisProcess.argv[0];
var score;
var setTransition = [\b_setn, 0, 0, 9] ++ [
	0.7, 0.2, 0.1,
	0.2, 0.6, 0.2,
	0.1, 0.3, 0.6
];
var setDurations = [\b_setn, 1, 0, 9] ++ [
	0.08, 1, 0,
	0.05, 2, 2,
	0.12, 0.7, 3
];
var setCovariance = [\b_setn, 2, 0, 9] ++ [
	1, 0.8, 0.4,
	0.8, 1, 0.6,
	0.4, 0.6, 1
];
var setLattice = [\b_setn, 3, 0, 8] ++ [0.2, 0.21, 0.22, 0.23, 0.24, 0.25, 0.26, 0.27];
var setValues = [\b_setn, 4, 0, 3] ++ [220, 330, 550];

score = Score([
	[0.0, [\b_alloc, 0, 9, 1]],
	[0.0, [\b_alloc, 1, 3, 3]],
	[0.0, [\b_alloc, 2, 9, 1]],
	[0.0, [\b_alloc, 3, 8, 1]],
	[0.0, [\b_alloc, 4, 3, 1]],
	[0.0, setTransition],
	[0.0, setDurations],
	[0.0, setCovariance],
	[0.0, setLattice],
	[0.0, setValues],
	[0.0, [\d_recv, SynthDef(\sdSmoke, { |out = 0, reset = 0|
		var fast = Impulse.ar(400);
		var medium = Impulse.kr(12);
		var maps, flows, processes, distributions, states, sonifiers, signal;

		maps = HenonMap.ar(fast, reset: reset)[0]
			+ GbmanMap.ar(fast, reset: reset)[1]
			+ LatoocarfianMap.ar(fast, reset: reset)[0]
			+ IkedaMap.ar(fast, reset: reset)[1]
			+ LoziMap.ar(fast, reset: reset)[0]
			+ TinkerbellMap.ar(fast, reset: reset)[1]
			+ CliffordMap.ar(fast, reset: reset)[0]
			+ DeJongMap.ar(fast, reset: reset)[1]
			+ CircleMap.ar(fast, reset: reset)
			+ CoupledLogisticMap.ar(fast, reset: reset)[0]
			+ Mix(CoupledMapLattice.ar(8, fast, initialBuf: 3, reset: reset))
			+ RulkovMap.ar(fast, reset: reset)[0];

		flows = MackeyGlass.ar(reset: reset)
			+ Mix(Chua.ar(timeScale: 3, reset: reset))
			+ (Izhikevich.ar(reset: reset)[2] * 10);

		processes = OUProcess.ar(seed: 11, reset: reset)
			+ (RenewalTrig.ar(20, seed: 12, reset: reset)[0] * 10)
			+ (HawkesTrig.ar(2, 8, 12, seed: 13, reset: reset)[0] * 10)
			+ FractionalNoise.ar(1, quality: 8, seed: 14, reset: reset)
			+ FBrownianMotion.kr(0.7, 0.2, seed: 15, reset: reset)
			+ BoundedWalk.ar(fast, seed: 16, reset: reset);

		distributions = TGammaRand.ar(2, 1, fast, 21, reset)
			+ TWeibullRand.ar(1.2, 1, fast, 22, reset)
			+ TLogNormalRand.ar(0, 0.2, fast, 23, reset)
			+ TCauchyRand.ar(0, 0.1, fast, 24, reset).clip2(2)
			+ TPoissonRand.ar(3, fast, 25, reset)
			+ TGeometricRand.ar(0.5, fast, 26, reset)
			+ TNegBinomialRand.ar(2, 0.6, fast, 27, reset)
			+ TTruncNormalRand.ar(0, 1, -1, 1, fast, 28, reset)
			+ TStableRand.ar(1.7, 0, 0.1, 0, fast, 29, reset).clip2(2)
			+ TLevyRand.ar(0, 0.02, fast, 30, reset).clip(0, 2);

		states = MarkovChain.kr(3, medium, 0, 4, seed: 31, reset: reset)[0]
			+ SemiMarkov.kr(3, 0, 1, 4, seed: 32, reset: reset)[0]
			+ Mix(MultiGaussNoise.kr(3, medium, 2, seed: 33, reset: reset));

		sonifiers = HenonSonify.ar(reset: reset)
			+ GbmanSonify.ar(reset: reset)
			+ LatoocarfianSonify.ar(reset: reset);

		signal = (maps.tanh * 0.02) + (flows.tanh * 0.02)
			+ (processes.tanh * 0.02) + (distributions.tanh * 0.02)
			+ (states.tanh * 0.02) + (sonifiers * 0.02);
		Out.ar(out, Limiter.ar(signal ! 2, 0.5) * Line.kr(0, 1, 0.02));
	}).asBytes]],
	[0.0, [\s_new, \sdSmoke, 1000, 0, 0]],
	[0.7, [\n_set, 1000, \reset, 1]],
	[0.72, [\n_set, 1000, \reset, 0]],
	[1.9, [\n_free, 1000]],
	[1.95, [\b_free, 0]],
	[1.95, [\b_free, 1]],
	[1.95, [\b_free, 2]],
	[1.95, [\b_free, 3]],
	[1.95, [\b_free, 4]],
	[2.0, [0]]
]);

score.writeOSCFile(oscPath, 0, 2.0);
("WROTE " ++ oscPath).postln;
0.exit;
)

