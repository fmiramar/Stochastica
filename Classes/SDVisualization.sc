SDPhaseView {
	var <window, <view, <synth, <responder, <trail;
	var xRange, yRange, title, maxPoints, fps, path;

	*open { |title = "State-space view", xRange = #[-1, 1], yRange = #[-1, 1],
		stateFunction, soundFunction, fps = 30, maxPoints = 1200|
		var instance = this.new;
		Server.default.waitForBoot {
			instance.init(title, xRange, yRange, stateFunction, soundFunction, fps, maxPoints)
		};
		^instance
	}

	init { |argTitle, argXRange, argYRange, stateFunction, soundFunction,
		argFps, argMaxPoints|
		var project, drawGrid, drawTrace;

		title = argTitle;
		xRange = argXRange;
		yRange = argYRange;
		fps = argFps.asInteger.clip(5, 60);
		maxPoints = argMaxPoints.asInteger.clip(60, 10000);
		path = ("/sd/phase/" ++ this.identityHash).asSymbol;
		trail = RingBuffer(maxPoints + 1);
		soundFunction = soundFunction ? { Silent.ar(2) };

		project = { |state, rect|
			Point(
				state[0].clip(xRange[0], xRange[1]).linlin(xRange[0], xRange[1], rect.left, rect.right),
				state[1].clip(yRange[0], yRange[1]).linlin(yRange[0], yRange[1], rect.bottom, rect.top)
			)
		};

		drawGrid = { |rect|
			Pen.strokeColor = Color(0.15, 0.19, 0.28);
			Pen.width = 1;
			7.do { |index|
				var fraction = (index + 1) / 8;
				var x = rect.left + (rect.width * fraction);
				var y = rect.top + (rect.height * fraction);
				Pen.line(Point(x, rect.top), Point(x, rect.bottom));
				Pen.line(Point(rect.left, y), Point(rect.right, y));
			};
			Pen.stroke;
		};

		drawTrace = { |rect|
			var latest, brightStart;
			Pen.strokeColor = Color(0.24, 0.35, 0.52, 0.72);
			Pen.width = 1;
			trail.do { |state, index|
				var point = project.(state, rect);
				if(index == 0) { Pen.moveTo(point) } { Pen.lineTo(point) };
				latest = point;
			};
			Pen.stroke;

			brightStart = (trail.size - 120).max(0);
			Pen.strokeColor = Color(0.78, 0.38, 0.88, 0.95);
			Pen.width = 1.6;
			trail.do { |state, index|
				var point = project.(state, rect);
				if(index == brightStart) { Pen.moveTo(point) };
				if(index > brightStart) { Pen.lineTo(point) };
			};
			Pen.stroke;

			if(latest.notNil) {
				Pen.fillColor = Color(1.0, 0.46, 0.24);
				Pen.fillOval(Rect.aboutPoint(latest, 4, 4));
			};
		};

		window = Window(title, Rect(100, 100, 760, 560));
		view = UserView(window, window.view.bounds)
			.resize_(5)
			.background_(Color(0.018, 0.022, 0.04))
			.frameRate_(fps)
			.animate_(true);
		view.drawFunc = { |canvas|
			var bounds = canvas.bounds;
			var plot = Rect(20, 48, bounds.width - 40, bounds.height - 68);
			Pen.fillColor = Color(0.035, 0.045, 0.075);
			Pen.fillRect(plot);
			drawGrid.(plot);
			drawTrace.(plot);
			Pen.color = Color(0.90, 0.94, 1.0);
			Pen.font = Font.sansSerif(18, true);
			Pen.stringAtPoint(title, 20 @ 14);
		};

		responder = OSCFunc({ |message|
			var sample = message[3..4];
			{ if(window.notNil) { trail.overwrite(sample) } }.defer;
		}, path, Server.default.addr);

		synth = {
			var state = stateFunction.value;
			SendReply.kr(Impulse.kr(fps), path, state[0..1]);
			soundFunction.value(state)
		}.play;

		window.onClose = { this.cleanup };
		window.front;
		CmdPeriod.doOnce({ { this.free }.defer });
		^this
	}

	cleanup {
		view.tryPerform(\animate_, false);
		responder.tryPerform(\free);
		synth.tryPerform(\free);
		view = nil;
		responder = nil;
		synth = nil;
		window = nil;
	}

	free {
		if(window.notNil) { window.close } { this.cleanup };
	}
}
