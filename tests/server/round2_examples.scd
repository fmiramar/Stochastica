(
var folder = thisProcess.argv[0];
var pluginFolder = thisProcess.argv[1];
var files = (folder +/+ "*.scd").pathMatch.sort;
var failed = false;
var timeout;
s = Server.local;
s.options.numOutputBusChannels = 4;
s.options.numInputBusChannels = 0;
s.options.loadDefs = false;
s.options.numWireBufs = 2048;
s.options.memSize = 65536;
s.options.ugenPluginsPath = [Platform.resourceDir +/+ "plugins", pluginFolder];
OSCdef(\round2Failure, { |msg| failed = true; msg.postln }, '/fail', s.addr);
OSCdef(\round2BadValue, { |msg| if(msg[3] > 0) { failed = true; msg.postln } }, '/round2Bad', s.addr);
timeout = Routine { 90.wait; "ROUND2_EXAMPLES_TIMEOUT".postln; s.quit; 1.exit }.play(AppClock);
s.waitForBoot {
    Routine {
        SynthDef(\round2Mute, {
            var signal = In.ar(0, 4);
            var bad = CheckBadValues.ar(signal, post: 0);
            SendReply.kr(Impulse.kr(20), '/round2Bad', A2K.kr(((bad > 0) * (bad < 3)).sum));
            ReplaceOut.ar(0, Silent.ar(4));
        }).add;
        s.sync;
        files.do { |file|
            var mute = Synth.tail(s.defaultGroup, \round2Mute);
            var result;
            ("RUNNING_EXAMPLE " ++ file.basename).postln;
            result = file.load;
            0.45.wait;
            if(result.isKindOf(Routine)) { result.stop };
            s.freeAll;
            s.sync;
        };
        timeout.stop;
        s.quit;
        if(failed) { "ROUND2_EXAMPLES_FAILED".postln; 1.exit } { "ROUND2_EXAMPLES_OK".postln; 0.exit };
    }.play(AppClock);
};
)
