(
var names = [
	"HenonMap", "GbmanMap", "LatoocarfianMap", "IkedaMap", "LoziMap",
	"TinkerbellMap", "CliffordMap", "DeJongMap", "CircleMap",
	"CoupledLogisticMap", "CoupledMapLattice", "RulkovMap", "MackeyGlass",
	"Chua", "Izhikevich", "OUProcess", "RenewalTrig", "HawkesTrig",
	"FractionalNoise", "FBrownianMotion", "BoundedWalk", "TGammaRand",
	"TWeibullRand", "TLogNormalRand", "TCauchyRand", "TPoissonRand",
	"TGeometricRand", "TNegBinomialRand", "TTruncNormalRand", "TStableRand",
	"TLevyRand", "MarkovChain", "SemiMarkov", "MultiGaussNoise",
	"HenonSonify", "GbmanSonify", "LatoocarfianSonify", "SDPhaseView"
];
var pages = names.collect { |name| "Classes/" ++ name } ++ ["Guides/StochasticDynamicsUGens"];
var ok = true;

SCDoc.verbosity = 2;
SCDoc.indexAllDocuments(true);

pages.do { |path|
	var doc = SCDoc.documents[path];
	if(doc.isNil) {
		("MISSING_DOC " ++ path).postln;
		ok = false;
	} {
		var root = SCDoc.parseDoc(doc);
		if(root.isNil) {
			("PARSE_FAILED " ++ path).postln;
			ok = false;
		} {
			SCDoc.renderer.renderToFile(doc.destPath, doc, root);
			("RENDERED " ++ path).postln;
		};
	};
};

if(ok) {
	"STOCHASTIC_DYNAMICS_SCDOC_OK".postln;
	0.exit;
} {
	"STOCHASTIC_DYNAMICS_SCDOC_FAILED".postln;
	1.exit;
};
)
