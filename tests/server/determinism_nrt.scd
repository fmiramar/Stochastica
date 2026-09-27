(
var oscPath = thisProcess.argv[0];
var score;
var transition = [
	0.7, 0.2, 0.1,
	0.2, 0.6, 0.2,
	0.1, 0.3, 0.6
];
var durations = [
	0.08, 1, 0,
	0.05, 2, 2,
	0.12, 0.7, 3
];
var covariance = [
	1, 0.8, 0.4,
	0.8, 1, 0.6,
	0.4, 0.6, 1
];

score = Score([
	[0.0, [\b_alloc, 0, 9, 1]],
	[0.0, [\b_alloc, 1, 3, 3]],
	[0.0, [\b_alloc, 2, 9, 1]],
	[0.0, [\b_setn, 0, 0, transition.size] ++ transition],
	[0.0, [\b_setn, 1, 0, durations.size] ++ durations],
	[0.0, [\b_setn, 2, 0, covariance.size] ++ covariance],
	[0.0, [\d_recv, SynthDef(\sdDeterminism, { |out = 0|
		var trig = Impulse.ar(800);
		var controlTrig = Impulse.kr(30);
		var reset = Impulse.ar(19);
		var differences;

		differences = [
			(OUProcess.ar(seed: 101, reset: reset) - OUProcess.ar(seed: 101, reset: reset)).abs,
			Mix((RenewalTrig.ar(30, seed: 102, reset: reset)
				- RenewalTrig.ar(30, seed: 102, reset: reset)).abs),
			Mix((HawkesTrig.ar(2, 8, 12, seed: 103, reset: reset)
				- HawkesTrig.ar(2, 8, 12, seed: 103, reset: reset)).abs),
			(FractionalNoise.ar(1.3, quality: 16, seed: 104, reset: reset)
				- FractionalNoise.ar(1.3, quality: 16, seed: 104, reset: reset)).abs,
			(FBrownianMotion.kr(0.8, seed: 105, reset: reset)
				- FBrownianMotion.kr(0.8, seed: 105, reset: reset)).abs,
			(BoundedWalk.ar(trig, seed: 106, reset: reset)
				- BoundedWalk.ar(trig, seed: 106, reset: reset)).abs,
			(TGammaRand.ar(2, 1, trig, 107, reset)
				- TGammaRand.ar(2, 1, trig, 107, reset)).abs,
			(TWeibullRand.ar(1.3, 1, trig, 108, reset)
				- TWeibullRand.ar(1.3, 1, trig, 108, reset)).abs,
			(TLogNormalRand.ar(0, 0.3, trig, 109, reset)
				- TLogNormalRand.ar(0, 0.3, trig, 109, reset)).abs,
			(TCauchyRand.ar(0, 1, trig, 110, reset)
				- TCauchyRand.ar(0, 1, trig, 110, reset)).abs,
			(TPoissonRand.ar(40, trig, 111, reset)
				- TPoissonRand.ar(40, trig, 111, reset)).abs,
			(TGeometricRand.ar(0.3, trig, 112, reset)
				- TGeometricRand.ar(0.3, trig, 112, reset)).abs,
			(TNegBinomialRand.ar(3, 0.4, trig, 113, reset)
				- TNegBinomialRand.ar(3, 0.4, trig, 113, reset)).abs,
			(TTruncNormalRand.ar(0, 1, -1, 1, trig, 114, reset)
				- TTruncNormalRand.ar(0, 1, -1, 1, trig, 114, reset)).abs,
			(TStableRand.ar(1.5, 0.2, 1, 0, trig, 115, reset)
				- TStableRand.ar(1.5, 0.2, 1, 0, trig, 115, reset)).abs,
			(TLevyRand.ar(0, 1, trig, 116, reset)
				- TLevyRand.ar(0, 1, trig, 116, reset)).abs,
			Mix((MarkovChain.kr(3, controlTrig, 0, seed: 117, reset: reset)
				- MarkovChain.kr(3, controlTrig, 0, seed: 117, reset: reset)).abs),
			Mix((SemiMarkov.kr(3, 0, 1, seed: 118, reset: reset)
				- SemiMarkov.kr(3, 0, 1, seed: 118, reset: reset)).abs),
			Mix((MultiGaussNoise.kr(3, controlTrig, 2, seed: 119, reset: reset)
				- MultiGaussNoise.kr(3, controlTrig, 2, seed: 119, reset: reset)).abs)
		];
		Out.ar(out, Mix(differences) ! 2);
	}).asBytes]],
	[0.0, [\s_new, \sdDeterminism, 1000, 0, 0]],
	[1.9, [\n_free, 1000]],
	[1.95, [\b_free, 0]],
	[1.95, [\b_free, 1]],
	[1.95, [\b_free, 2]],
	[2.0, [0]]
]);

score.writeOSCFile(oscPath, 0, 2.0);
("WROTE " ++ oscPath).postln;
0.exit;
)

