(
var graphs = [
    {
        var state = HenonMap.kr(Impulse.kr(90));
        Pan2.ar(SinOsc.ar(state[0].clip(-1.5, 1.5).linexp(-1.5, 1.5, 100, 1800), 0, 0.06), state[1].clip(-0.5, 0.5).linlin(-0.5, 0.5, -1, 1))
    },
    {
        var state = GbmanMap.kr(Impulse.kr(45));
        Pan2.ar(SinOsc.ar(state[0].clip(-4, 8).linexp(-4, 8, 100, 1800), 0, 0.06), state[1].clip(-4, 8).linlin(-4, 8, -1, 1))
    },
    {
        var state = LatoocarfianMap.kr(Impulse.kr(120));
        Pan2.ar(SinOsc.ar(state[0].clip(-1.6, 1.6).linexp(-1.6, 1.6, 100, 1800), 0, 0.06), state[1].clip(-1.6, 1.6).linlin(-1.6, 1.6, -1, 1))
    },
    {
        var state = IkedaMap.kr(Impulse.kr(80));
        Pan2.ar(SinOsc.ar(state[0].clip(-0.5, 1.9).linexp(-0.5, 1.9, 100, 1800), 0, 0.06), state[1].clip(-2.4, 1).linlin(-2.4, 1, -1, 1))
    },
    {
        var state = LoziMap.kr(Impulse.kr(100));
        Pan2.ar(SinOsc.ar(state[0].clip(-1.5, 1.5).linexp(-1.5, 1.5, 100, 1800), 0, 0.06), state[1].clip(-1.5, 1.5).linlin(-1.5, 1.5, -1, 1))
    },
    {
        var state = TinkerbellMap.kr(Impulse.kr(120));
        Pan2.ar(SinOsc.ar(state[0].clip(-1.4, 0.7).linexp(-1.4, 0.7, 100, 1800), 0, 0.06), state[1].clip(-1.8, 0.8).linlin(-1.8, 0.8, -1, 1))
    },
    {
        var state = CliffordMap.kr(Impulse.kr(120));
        Pan2.ar(SinOsc.ar(state[0].clip(-1.6, 2.1).linexp(-1.6, 2.1, 100, 1800), 0, 0.06), state[1].clip(-1.5, 1.7).linlin(-1.5, 1.7, -1, 1))
    },
    {
        var state = DeJongMap.kr(Impulse.kr(120));
        Pan2.ar(SinOsc.ar(state[0].clip(-2.1, 2.1).linexp(-2.1, 2.1, 100, 1800), 0, 0.06), state[1].clip(-2.1, 2.1).linlin(-2.1, 2.1, -1, 1))
    },
    {
        var state = CoupledLogisticMap.kr(Impulse.kr(90));
        Pan2.ar(SinOsc.ar(state[0].clip(0, 1).linexp(0, 1, 100, 1800), 0, 0.06), state[1].clip(0, 1).linlin(0, 1, -1, 1))
    },
    {
        var state = RulkovMap.kr(Impulse.kr(600));
        var spike = HPZ1.kr(state[0] > 0).max(0);
        Pan2.ar(Decay2.ar(K2A.ar(spike), 0.001, 0.035) * SinOsc.ar(700, 0, 0.09), state[1].clip(-3.05, -2.65).linlin(-3.05, -2.65, -1, 1))
    },
    {
        var state = Izhikevich.kr(input: 12);
        Pan2.ar(Decay2.ar(K2A.ar(state[2]), 0.001, 0.05) * SinOsc.ar(800, 0, 0.1), state[0].clip(-70, 32).linlin(-70, 32, -1, 1))
    }
];

graphs.do { |graph, index|
    SynthDef(("SDPhaseExample" ++ index).asSymbol, { Out.ar(0, graph.value) }).asBytes;
};
"STOCHASTIC_DYNAMICS_PHASE_EXAMPLES_OK".postln;
0.exit;
)
