(
var path = thisProcess.argv[0];
var duration = (thisProcess.argv[1] ? "600").asFloat;
var score = List.new;
var definition;
var arrays = [
    Array.fill(1024, { |i| if((i div: 32) == (i % 32), 1, 0.4) }),
    1 ! 16, 0.1 ! 256, 10 ! 256, Array.series(64, 0.8, 0.006)
];
arrays.do { |values, i|
    score.add([0, [\b_alloc, i, values.size, 1]]);
    score.add([0.01, [\b_setn, i, 0, values.size] ++ values]);
};
definition = SynthDef(\round2Endurance, {
    var reset = Impulse.kr(7);
    var reload = Impulse.kr(3);
    var state = [
        MultiHawkesTrig.ar(16, 1, 2, 3, reload: reload, seed: 11, reset: reset),
        MultiOUProcess.ar(32, 0, reload: reload, seed: 12, reset: reset),
        KuramotoNetwork.ar(64, 4, coupling: 4, noise: 0.2, substeps: 1, reload: reload, seed: 13, reset: reset),
        KuramotoNetwork.kr(64, 4, coupling: 4, noise: 0.2, substeps: 16, reload: reload, seed: 14, reset: reset),
        DoubleWellProcess.ar(SinOsc.ar(20), noise: 2, timeScale: 100, substeps: 16, seed: 15, reset: reset),
        RandomTelegraph.ar(upRate: 1e20, downRate: 1e20, seed: 16, reset: reset),
        ShotNoise.ar(1e20, 0.001, 0, 1, 17, reset),
        SelfCorrectingTrig.ar(SinOsc.ar(30).range(0, 20000), 10, seed: 18, reset: reset),
        TVonMisesRand.ar(kappa: 1e20, trig: Impulse.ar(24000), seed: 19, reset: reset),
        MultiOUProcess.kr(32, 999999, reload: reload),
        MultiHawkesTrig.kr(16, -1, 999999, 0, reload: reload),
        KuramotoNetwork.kr(64, 999999, 0, substeps: 16, reload: reload)
    ].flat.collect { |x| if(x.rate == \audio, { x }, { K2A.ar(x) }) };
    var bad = CheckBadValues.ar(state, post: 0);
    Out.ar(0, [Mix(state).tanh * 0.01, ((bad > 0) * (bad < 3)).sum]);
});
definition.writeDefFile(path.dirname);
score.add([0, [\d_load, (path.dirname +/+ "round2Endurance.scsyndef").standardizePath]]);
score.add([0.128, [\s_new, \round2Endurance, 1000, 0, 0]]);
score.add([duration-0.01, [\n_free, 1000]]);
score.add([duration, [0]]);
Score(score.asArray.sort { |a,b| a[0] < b[0] }).writeOSCFile(path, 0, duration);
"ROUND2_ENDURANCE_SCORE_OK".postln;
0.exit;
)
