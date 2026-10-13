(
var path = thisProcess.argv[0];
var score = List.new;
var arrays = [[0.7,0.2,0.1, 0.2,0.6,0.2, 0.1,0.3,0.6],
    [0.03,1,0, 0.05,2,2, 0.02,0.7,3], [1,0.6,0.2, 0.6,1,0.4, 0.2,0.4,1]];
arrays.do { |values,i|
    score.add([0, [\b_alloc, i, if(i==1,3,9), if(i==1,3,1)]]);
    score.add([0.01, [\b_setn, i, 0, 9] ++ values]);
};
score.add([0,[\d_recv,SynthDef(\round3Legacy, { |reset=0|
    var signals = [\ar,\kr].collect { |rate|
        var trig = Impulse.perform(rate, if(rate==\ar,4000,200));
        var signal = [
            TGammaRand.perform(rate,0.4,1,trig,11,reset),
            TStableRand.perform(rate,1.4,0.2,1,0,trig,12,reset),
            TPoissonRand.perform(rate,40,trig,13,reset),
            MarkovChain.perform(rate,3,trig,0,-1,0,14,reset),
            SemiMarkov.perform(rate,3,0,1,-1,0,1,15,reset),
            OUProcess.perform(rate,0,1,1,1,0,16,reset),
            MultiGaussNoise.perform(rate,3,trig,2,-1,0,17,reset)
        ].flat;
        if(rate==\ar,{signal},{K2A.ar(signal)})
    }.flat;
    Out.ar(0,signals);
}).asBytes]]);
score.add([0.128,[\s_new,\round3Legacy,1000,0,0]]);
score.add([0.512,[\n_set,1000,\reset,1]]);
score.add([0.516,[\n_set,1000,\reset,0]]);
score.add([1.99,[\n_free,1000]]);
score.add([2,[0]]);
Score(score.asArray.sort { |a,b| a[0]<b[0] }).writeOSCFile(path,0,2);
"ROUND3_LEGACY_SCORE_OK".postln;
0.exit;
)
