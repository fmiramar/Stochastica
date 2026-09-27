(
var oscPath = thisProcess.argv[0];
var covariance = Array.fill(32 * 32, { |index| if((index div: 32) == (index % 32)) { 1 } { 0 } });
var lattice = Array.fill(64, { |index| 0.2 + (index * 0.001) });
var score;

score = Score([
	[0.0, [\b_alloc, 0, 1024, 1]],
	[0.0, [\b_alloc, 1, 64, 1]],
	[0.0, [\b_setn, 0, 0, covariance.size] ++ covariance],
	[0.0, [\b_setn, 1, 0, lattice.size] ++ lattice],
	[0.0, [\d_recv, SynthDef(\sdStress, { |out = 0|
		var dense = Impulse.ar(SampleRate.ir);
		var reset = Impulse.ar(137);
		var maps, samplers, memory, missing, signal;

		maps = HenonMap.ar(dense, 1.4, 0.3, reset: reset)[0]
			+ IkedaMap.ar(dense, reset: reset)[1]
			+ TinkerbellMap.ar(dense, reset: reset)[0]
			+ RulkovMap.ar(dense, reset: reset)[0]
			+ Mix(CoupledMapLattice.ar(64, dense, 3.8, 0.12, 1, reset));

		samplers = TGammaRand.ar(0.1, 10, dense, 101, reset)
			+ TPoissonRand.ar(1000, dense, 102, reset)
			+ TTruncNormalRand.ar(0, 10, -0.01, 0.01, dense, 103, reset)
			+ TStableRand.ar(0.2, 1, 0.01, 0, dense, 104, reset).clip2(10)
			+ TLevyRand.ar(0, 0.001, dense, 105, reset).clip(0, 10);

		memory = FractionalNoise.ar(2, 0.01, 20000, 32, 106, reset)
			+ FBrownianMotion.kr(0.99, 1, 0, 4096, 107, reset)
			+ Mix(MultiGaussNoise.kr(32, Impulse.kr(100), 0, reload: Impulse.kr(17), seed: 108, reset: reset));

		missing = MarkovChain.ar(256, dense, 9999, seed: 109, reset: reset)[0]
			+ SemiMarkov.ar(256, 9999, 9998, seed: 110, reset: reset)[0];

		signal = (maps.tanh + samplers.tanh + memory.tanh + missing.tanh) * 0.02;
		Out.ar(out, Limiter.ar(signal ! 2, 0.5));
	}).asBytes]],
	[0.0, [\s_new, \sdStress, 1000, 0, 0]],
	[3.9, [\n_free, 1000]],
	[3.95, [\b_free, 0]],
	[3.95, [\b_free, 1]],
	[4.0, [0]]
]);

score.writeOSCFile(oscPath, 0, 4.0);
("WROTE " ++ oscPath).postln;
0.exit;
)

