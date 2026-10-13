(
var folder = thisProcess.argv[0];
var pluginFolder = thisProcess.argv[1];
var files = (folder +/+ "*.scd").pathMatch.sort;
var failed = false;
var timeout;
if(thisProcess.argv[2].notNil) { Server.program = thisProcess.argv[2] };
s = Server(\gendyExamples, NetAddr("127.0.0.1", (thisProcess.argv[3] ? "57239").asInteger));
Server.default = s;
s.options.numOutputBusChannels = 4;
s.options.numInputBusChannels = 0;
s.options.loadDefs = false;
s.options.numWireBufs = 2048;
s.options.memSize = 65536;
s.options.ugenPluginsPath = [Platform.resourceDir +/+ "plugins", pluginFolder];
OSCdef(\gendyFailure, { |msg| failed = true; msg.postln }, '/fail', s.addr);
OSCdef(\gendyBadValue, { |msg| if(msg[3] > 0) { failed = true; msg.postln } }, '/gendyBad', s.addr);
timeout = Routine { 120.wait; "GENDY_EXAMPLES_TIMEOUT".postln; s.quit; 1.exit }.play(AppClock);
s.waitForBoot {
    Routine {
        SynthDef(\gendyMute, {
            var signal = In.ar(0, 4);
            var bad = CheckBadValues.ar(signal, post: 0);
            SendReply.kr(Impulse.kr(20), '/gendyBad', A2K.kr(((bad > 0) * (bad < 3)).sum));
            ReplaceOut.ar(0, Silent.ar(4));
        }).add;
        s.sync;
        files.do { |file|
            var mute = Synth.tail(s.defaultGroup, \gendyMute);
            var result;
            ("RUNNING_EXAMPLE " ++ file.basename).postln;
            result = file.load;
            3.wait;
            if(result.isKindOf(Routine)) { result.stop };
            s.freeAll;
            s.sync;
        };
        timeout.stop;
        s.quit;
        if(failed) { "GENDY_EXAMPLES_FAILED".postln; 1.exit } { "GENDY_EXAMPLES_OK".postln; 0.exit };
    }.play(AppClock);
};
)
