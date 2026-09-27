(
var phaseView;

Server.default.waitForBoot {
    phaseView = SDPhaseView.open(
        "SDPhaseView smoke test",
        [-1.5, 1.5], [-0.5, 0.5],
        { HenonMap.kr(Impulse.kr(90)) },
        { |state|
            Pan2.ar(
                SinOsc.ar(state[0].clip(-1.5, 1.5).linexp(-1.5, 1.5, 100, 1800), 0, 0.01),
                state[1].clip(-0.5, 0.5).linlin(-0.5, 0.5, -1, 1)
            )
        },
        30,
        180
    );

    Routine {
        2.wait;
        if(phaseView.window.isNil or: { phaseView.synth.isNil } or: { phaseView.responder.isNil }) {
            "STOCHASTIC_DYNAMICS_PHASE_VIEW_FAILED".postln;
            1.exit;
        };
        phaseView.free;
        0.25.wait;
        if(phaseView.window.notNil or: { phaseView.synth.notNil } or: { phaseView.responder.notNil }) {
            "STOCHASTIC_DYNAMICS_PHASE_VIEW_CLEANUP_FAILED".postln;
            1.exit;
        };
        "STOCHASTIC_DYNAMICS_PHASE_VIEW_OK".postln;
        Server.default.quit;
        0.exit;
    }.play(AppClock);
};
)
