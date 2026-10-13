(
var oscPath = thisProcess.argv[0];
var duration = (thisProcess.argv[1] ? "2").asFloat;
var stress = duration > 5;
var score = List.new;
var arrays = [
    [1, 0.7, 0.7, 1], [0.1, -0.2], [1, 2], [2, 0.2],
    [0, 0, 12, 0], [10, 10, 10, 10], [0.9, 1.1], [-1, 1],
    Array.fill(1024, { |i| if((i div: 32) == (i % 32), 1, 0.4) }),
    1 ! 16, 0.1 ! 256, 10 ! 256, Array.series(64, 0.8, 0.006)
];
var definition;
arrays.do { |values, index|
    score.add([0, [\b_alloc, index, values.size, 1]]);
    score.add([0.01, [\b_setn, index, 0, values.size] ++ values]);
};
definition = SynthDef(\round2Integration, { |reset = 0, reload = 0|
    var outputs = List.new;
    [\ar, \kr].do { |method|
        var trigger = if(method == \ar, { Impulse.ar(800) }, { Impulse.kr(30) });
        var build = {
            [
                SelfCorrectingTrig.perform(method, 6, 1.5, 20000, 123, reset),
                RandomTelegraph.perform(method, -1, 1, 3, 1, 0, 123, reset),
                ShotNoise.perform(method, 20, 0.05, 0.4, 0.2, 123, reset),
                TVonMisesRand.perform(method, 3.13, 2, trigger, 123, reset),
                MultiOUProcess.perform(method, 2, 0, 1, 2, reload, 123, reset),
                MultiHawkesTrig.perform(method, 2, 3, 4, 5, 20000, reload, 123, reset),
                DoubleWellProcess.perform(method, 0, 1, 1, 0.8, 4, -1, 4, 123, reset),
                KuramotoNetwork.perform(method, 2, 6, 7, 3, 0.2, 1, 4, reload, 123, reset)
            ].flat
        };
        var first = build.value;
        var second = build.value;
        if(first.size != 18) { Error("Round 2 output count").throw };
        outputs.addAll(if(method == \ar, { first }, { K2A.ar(first) }));
        outputs.addAll(if(method == \ar, { first - second }, { K2A.ar(first - second) }));
    };
    Out.ar(0, outputs.asArray);
});
score.add([0, [\d_recv, definition.asBytes]]);
score.add([0.128, [\s_new, \round2Integration, 1000, 0, 0]]);
score.add([0.512, [\n_set, 1000, \reset, 1, \reload, 1]]);
score.add([0.5146666666667, [\n_set, 1000, \reset, 0, \reload, 0]]);
score.add([1.024, [\n_set, 1000, \reset, 1]]);
score.add([1.0266666666667, [\n_set, 1000, \reset, 0]]);
if(stress) {
    score.add([0, [\d_recv, SynthDef(\round2Stress, {
        var reset = Impulse.kr(7);
        var reload = Impulse.kr(3);
        var out = [
            MultiHawkesTrig.ar(16, 9, 10, 11, 20000, reload, 400, reset),
            MultiOUProcess.ar(32, 8, reload: reload, seed: 401, reset: reset),
            KuramotoNetwork.ar(64, 12, coupling: 8, noise: 0.2, substeps: 16, reload: reload, seed: 402, reset: reset),
            DoubleWellProcess.ar(SinOsc.ar(20), noise: 2, timeScale: 100, substeps: 16, seed: 403, reset: reset),
            RandomTelegraph.ar(upRate: 1e20, downRate: 1e20, seed: 404, reset: reset),
            ShotNoise.ar(1e20, 0.001, 1, 1, 405, reset),
            SelfCorrectingTrig.ar(SinOsc.ar(30).range(0, 20000), 10, seed: 406, reset: reset),
            TVonMisesRand.ar(kappa: 1e20, trig: Impulse.ar(24000), seed: 407, reset: reset),
            MultiOUProcess.kr(32, 999999, reload: reload),
            MultiHawkesTrig.kr(16, -1, 999999, 0, reload: reload),
            KuramotoNetwork.kr(64, 999999, 0, substeps: 16, reload: reload)
        ].flat;
        Out.ar(72, Mix(out.collect { |x| if(x.rate == \audio, { x }, { K2A.ar(x) }) }).tanh);
    }).asBytes]]);
    score.add([0.128, [\s_new, \round2Stress, 1001, 0, 0]]);
    100.do { |i|
        score.add([2 + (i * 0.02), [\s_new, \round2Integration, 2000+i, 0, 0]]);
        score.add([2.01 + (i * 0.02), [\n_free, 2000+i]]);
    };
    score.add([duration-0.01, [\n_free, 1001]]);
};
score.add([duration-0.01, [\n_free, 1000]]);
score.add([duration, [0]]);
Score(score.asArray.sort { |a,b| a[0] < b[0] }).writeOSCFile(oscPath, 0, duration);
"ROUND2_SCORE_OK".postln;
0.exit;
)
