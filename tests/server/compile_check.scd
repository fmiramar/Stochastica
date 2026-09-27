(
var classes = [
	HenonMap, GbmanMap, LatoocarfianMap, IkedaMap, LoziMap, TinkerbellMap,
	CliffordMap, DeJongMap, CircleMap, CoupledLogisticMap, CoupledMapLattice,
	RulkovMap, MackeyGlass, Chua, Izhikevich, OUProcess, RenewalTrig,
	HawkesTrig, FractionalNoise, FBrownianMotion, BoundedWalk, TGammaRand,
	TWeibullRand, TLogNormalRand, TCauchyRand, TPoissonRand, TGeometricRand,
	TNegBinomialRand, TTruncNormalRand, TStableRand, TLevyRand, MarkovChain,
	SemiMarkov, MultiGaussNoise, HenonSonify, GbmanSonify, LatoocarfianSonify
];
var builders;

builders = [
	{ HenonMap.ar }, { HenonMap.kr }, { GbmanMap.ar }, { GbmanMap.kr },
	{ LatoocarfianMap.ar }, { LatoocarfianMap.kr }, { IkedaMap.ar }, { IkedaMap.kr },
	{ LoziMap.ar }, { LoziMap.kr }, { TinkerbellMap.ar }, { TinkerbellMap.kr },
	{ CliffordMap.ar }, { CliffordMap.kr }, { DeJongMap.ar }, { DeJongMap.kr },
	{ CircleMap.ar }, { CircleMap.kr }, { CoupledLogisticMap.ar }, { CoupledLogisticMap.kr },
	{ CoupledMapLattice.ar }, { CoupledMapLattice.kr }, { RulkovMap.ar }, { RulkovMap.kr },
	{ MackeyGlass.ar }, { MackeyGlass.kr }, { Chua.ar }, { Chua.kr },
	{ Izhikevich.ar }, { Izhikevich.kr }, { OUProcess.ar }, { OUProcess.kr },
	{ RenewalTrig.ar }, { RenewalTrig.kr }, { HawkesTrig.ar }, { HawkesTrig.kr },
	{ FractionalNoise.ar }, { FractionalNoise.kr }, { FBrownianMotion.kr },
	{ BoundedWalk.ar }, { BoundedWalk.kr }, { TGammaRand.ar }, { TGammaRand.kr },
	{ TWeibullRand.ar }, { TWeibullRand.kr }, { TLogNormalRand.ar }, { TLogNormalRand.kr },
	{ TCauchyRand.ar }, { TCauchyRand.kr }, { TPoissonRand.ar }, { TPoissonRand.kr },
	{ TGeometricRand.ar }, { TGeometricRand.kr }, { TNegBinomialRand.ar },
	{ TNegBinomialRand.kr }, { TTruncNormalRand.ar }, { TTruncNormalRand.kr },
	{ TStableRand.ar }, { TStableRand.kr }, { TLevyRand.ar }, { TLevyRand.kr },
	{ MarkovChain.ar }, { MarkovChain.kr }, { SemiMarkov.ar }, { SemiMarkov.kr },
	{ MultiGaussNoise.ar }, { MultiGaussNoise.kr }, { HenonSonify.ar },
	{ GbmanSonify.ar }, { LatoocarfianSonify.ar }
];

classes.do(_.name.postln);
builders.do { |builder, index|
	SynthDef(("sdCompile" ++ index).asSymbol, {
		builder.value;
		DC.ar(0)
	}).asBytes;
};

if(HenonMap.ar.size != 2) { Error("HenonMap output count").throw };
if(Chua.ar.size != 3) { Error("Chua output count").throw };
if(Izhikevich.ar.size != 3) { Error("Izhikevich output count").throw };
if(RenewalTrig.ar.size != 2) { Error("RenewalTrig output count").throw };
if(SemiMarkov.kr.size != 4) { Error("SemiMarkov output count").throw };
if(CoupledMapLattice.ar(8).size != 8) { Error("CoupledMapLattice output count").throw };
if(MultiGaussNoise.kr(7).size != 7) { Error("MultiGaussNoise output count").throw };

"STOCHASTIC_DYNAMICS_CLASS_LIBRARY_OK".postln;
0.exit;
)

