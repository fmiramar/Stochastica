(
var oscPath = thisProcess.argv[0];
var duration = (thisProcess.argv[1] ? "2").asFloat;
var mode = thisProcess.argv[2] ? "integration";
var score = List.new;
var arrays = [
    [1, 2, 3], [1, 1, 1], [110, 220, 330],
    [0, 1, 0, 0.3, 0, 0.7, 1, 0, 0], [1, 8, 25], [2, 1, 3],
    [8.01] ++ (8 ! 7), 0.2 ! 32, 1 ! 64,
    1 ! 4096, 12000 ! 64, 12000 ! 64, [8.01] ++ (8 ! 63)
];
var definition;
arrays.do { |values, index|
    score.add([0, [\b_alloc, index, values.size, 1]]);
    // Keep OSC messages below the normal UDP packet limit as well as valid in NRT.
    values.clump(512).do { |chunk, part|
        score.add([0.01, [\b_setn, index, part * 512, chunk.size] ++ chunk]);
    };
};
definition = SynthDef(\round3Integration, { |reset = 0, reload = 0|
    var outputs = List.new;
    [\ar, \kr].do { |method|
        var trigger = if(method == \ar, { Impulse.ar(800) }, { Impulse.kr(30) });
        var build = {
            [
                TDirichletRand.perform(method, 3, 0, trigger, reload, 123, reset),
                PolyaUrn.perform(method, 3, 1, trigger, 1, 2, reload, 123, reset),
                MarkovModulatedTrig.perform(method, 3, 3, 4, 5, 0, 20000, 20000, reload, 123, reset),
                MaternNoise.perform(method, 0.3, 2, 0.02, 2, 1, 0, 123, reset),
                LevyFlight.perform(method, 1.3, 0.5, 0.1, 1, 0, 123, reset),
                JumpDiffusion.perform(method, 0.1, 0.5, 5, 0.3, 0.8, 1, 0, 123, reset),
                WilsonCowan.perform(method, 0, 0, 10, 12, 10, 2, 0.02, 0.01, 1, 0, 1, 0, 1, 4, 0.1, 0.1, reset),
                Lorenz96.perform(method, 8, 8, 1, 4, 6, reload, reset)
            ].flat
        };
        var first = build.value;
        var second = build.value;
        if(first.size != 28) { Error("Round 3 output count").throw };
        outputs.addAll(if(method == \ar, { first }, { K2A.ar(first) }));
        outputs.addAll(if(method == \ar, { first - second }, { K2A.ar(first - second) }));
    };
    if(mode == "endurance") {
        var raw = outputs.asArray;
        var bad = CheckBadValues.ar(raw, post: 0);
        Out.ar(0, [Mix(raw.collect(_.tanh)) / raw.size, ((bad > 0) * (bad < 3)).sum]);
    } {
        Out.ar(0, outputs.asArray);
    };
});
score.add([0, [\d_recv, definition.asBytes]]);
score.add([0.128, [\s_new, \round3Integration, 1000, 0, 0]]);
score.add([0.512, [\n_set, 1000, \reset, 1, \reload, 1]]);
score.add([0.5146666666667, [\n_set, 1000, \reset, 0, \reload, 0]]);
score.add([1.024, [\n_set, 1000, \reset, 1]]);
score.add([1.0266666666667, [\n_set, 1000, \reset, 0]]);
if(mode == "stress") {
    score.add([0, [\d_recv, SynthDef(\round3Stress, {
        var reset = Impulse.kr(7), reload = Impulse.kr(3);
        var out = [
            TDirichletRand.ar(32, 7, Impulse.ar(24000), reload, 70, reset),
            PolyaUrn.ar(64, 8, Impulse.ar(24000), 100, reload: reload, seed: 71, reset: reset),
            MarkovModulatedTrig.ar(64, 9, 10, 11, reload: reload, seed: 72, reset: reset),
            Lorenz96.ar(64, 8, 1, 16, 12, reload, reset),
            Array.fill(8, { |i| WilsonCowan.ar(i / 8, substeps: 16, reset: reset) }),
            MaternNoise.ar(variance: 1e20, correlationTime: 1e-6, smoothness: 2, reset: reset),
            LevyFlight.ar(0.01, 1e20, reset: reset),
            JumpDiffusion.ar(jumpRate: 1e30, reset: reset),
            TDirichletRand.kr(32, 999999, Impulse.kr(30), reload, 73, reset),
            PolyaUrn.kr(64, -1, Impulse.kr(30), reload: reload, reset: reset),
            MarkovModulatedTrig.kr(64, -1, 999999, 0, reload: reload, reset: reset),
            Lorenz96.kr(64, initialBuf: 999999, reload: reload, reset: reset)
        ].flat;
        var raw = out.collect { |x| if(x.rate == \audio, { x }, { K2A.ar(x) }) };
        var bad = CheckBadValues.ar(raw, post: 0);
        Out.ar(112, ((bad > 0) * (bad < 3)).sum);
    }).asBytes]]);
    score.add([0.128, [\s_new, \round3Stress, 1001, 0, 0]]);
    100.do { |i|
        score.add([2 + (i * 0.02), [\s_new, \round3Integration, 2000+i, 0, 0]]);
        score.add([2.01 + (i * 0.02), [\n_free, 2000+i]]);
    };
    score.add([duration-0.01, [\n_free, 1001]]);
};
score.add([duration-0.01, [\n_free, 1000]]);
score.add([duration, [0]]);
Score(score.asArray.sort { |a,b| a[0] < b[0] }).writeOSCFile(oscPath, 0, duration);
"ROUND3_SCORE_OK".postln;
0.exit;
)
