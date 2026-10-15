(
var oscPath = thisProcess.argv[0];
var duration = (thisProcess.argv[1] ? "3").asFloat;
var mode = thisProcess.argv[2] ? "integration";
var score = List.new;
var definition = SynthDef(\gendyIntegration, { |reset = 0|
    var build = {
        [Gendy1977.ar(seed: 123, reset: reset), Gendy1991.ar(seed: 123, reset: reset),
            GendyS709.ar(seed: 123, reset: reset), GendyPerc.ar(reset, seed: 123),
            GendyInterp.ar(seed: 123, reset: reset), GendyCoupled.ar(seed: 123, reset: reset),
            GendyDiffusion.ar(seed: 123, reset: reset)]
            ++ K2A.ar(GendyField.kr(4, 0.1, 0.5, reset, 123))
    };
    var first = build.value, second = build.value;
    var baseline = Gendy1977.ar(seed: 999, reset: reset);
    var zero = [GendyCoupled.ar(0, seed: 999, reset: reset),
        GendyInterp.ar(seed: 999, reset: reset),
        GendyS709.ar(ampStepDepth: 0, durStepDepth: 0, seed: 999, reset: reset)];
    var out = first ++ (first - second) ++ (zero - baseline);
    if(mode == "endurance") {
        var bad = CheckBadValues.ar(out, post: 0);
        Out.ar(0, [Mix(out) / out.size, ((bad > 0) * (bad < 3)).sum]);
    } {
        Out.ar(0, out);
    };
});
score.add([0, [\d_recv, definition.asBytes]]);
score.add([0.128, [\s_new, \gendyIntegration, 1000, 0, 0]]);
score.add([0.512, [\n_set, 1000, \reset, 1]]);
score.add([0.516, [\n_set, 1000, \reset, 0]]);
score.add([1.024, [\n_set, 1000, \reset, 1]]);
score.add([1.028, [\n_set, 1000, \reset, 0]]);
if(mode == "stress") {
    score.add([0, [\d_recv, SynthDef(\gendyStress, {
        var reset = Impulse.ar(19);
        var waveforms = [2,64].collect { |n|
            [Gendy1977.ar(minFreq: 1e20, maxFreq: 1e20, numPoints: n, reset: reset, seed: 51),
                Gendy1991.ar(minFreq: 1e20, maxFreq: 1e20, numPoints: n, reset: reset, seed: 52),
                GendyS709.ar(modFreq: 1e20, numPoints: n, reset: reset, seed: 53),
                GendyPerc.ar(reset, 0, numPoints: n, seed: 54),
                GendyInterp.ar(LFNoise0.kr(100).range(0,3), minFreq: 1e20, maxFreq: 1e20, numPoints: n, reset: reset, seed: 55),
                GendyCoupled.ar(0.5, 0.5, numPoints: n, reset: reset, seed: 56),
                GendyDiffusion.ar(1e20, 10, 16, 0.01, 1e20, n, reset, 58)]
        }.flat;
        var raw = waveforms ++ K2A.ar(GendyField.kr(32, 0, 0.5, Impulse.kr(17), 57));
        var bad = CheckBadValues.ar(raw, post: 0);
        Out.ar(25, ((bad > 0) * (bad < 3)).sum);
    }).asBytes]]);
    score.add([0.128, [\s_new, \gendyStress, 1001, 0, 0]]);
    100.do { |i|
        score.add([2 + (i * 0.02), [\s_new, \gendyStress, 2000+i, 0, 0]]);
        score.add([2.01 + (i * 0.02), [\n_free, 2000+i]]);
    };
    score.add([duration-0.01, [\n_free, 1001]]);
};
score.add([duration-0.01, [\n_free, 1000]]);
score.add([duration, [0]]);
Score(score.asArray.sort { |a,b| a[0] < b[0] }).writeOSCFile(oscPath, 0, duration);
"GENDY_SCORE_OK".postln;
0.exit;
)
