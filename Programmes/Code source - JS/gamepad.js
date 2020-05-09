var controllers = {};

function addgamepad(gamepad) {
	controllers[gamepad.index] = gamepad; var d = document.createElement("div");
	d.setAttribute("id", "controller" + gamepad.index);
	var t = document.createElement("h1");
	t.appendChild(document.createTextNode("gamepad: " + gamepad.id));
	d.appendChild(t);
	var b = document.createElement("div");
	b.className = "buttons";
	for (var i=0; i<gamepad.buttons.length; i++) {
		var e = document.createElement("span");
		e.className = "button";
		e.innerHTML = i;
		b.appendChild(e);
	}

	d.appendChild(b);
	var a = document.createElement("div");
	a.className = "axes";
	for (i=0; i<gamepad.axes.length; i++) {
		e = document.createElement("meter");
		e.className = "axis";
		//e.id = "a" + i;
		e.setAttribute("min", "-1");
		e.setAttribute("max", "1");
		e.setAttribute("value", "0");
		e.innerHTML = i;
		a.appendChild(e);
	}

	d.appendChild(a);
	document.getElementById("start").style.display = "none";
	document.body.appendChild(d);
	requestAnimationFrame(updateStatus);
}

function disconnecthandler(e) {
	removegamepad(e.gamepad);
}

function removegamepad(gamepad) {
	var d = document.getElementById("controller" + gamepad.index);
	document.body.removeChild(d);
	delete controllers[gamepad.index];
}

var keys = [], prevKeys = [];

function updateStatus() {
	scangamepads();
	keys = [];
	for (j in controllers) {
		var controller = controllers[j];
		var d = document.getElementById("controller" + j);
		var buttons = d.getElementsByClassName("button");
		for (var i=0; i<controller.buttons.length; i++) {
			var b = buttons[i];
			var val = controller.buttons[i];
			var pressed = val == 1.0;
			if (typeof(val) == "object") {
				pressed = val.pressed;
				val = val.value;
			}
			if(pressed){
				DebugPrint(i);
				keys.push(i, keys, prevKeys);
			}
			handleEvent(i, keys, prevKeys);
			var pct = Math.round(val * 100) + "%";
			b.style.backgroundSize = pct + " " + pct;
			if (pressed)
				b.className = "button pressed";
			else
				b.className = "button";
		}

		var axes = d.getElementsByClassName("axis");
		for (var i=0; i<controller.axes.length; i++) {
			var a = axes[i];
			a.innerHTML = i + ": " + controller.axes[i].toFixed(4);
			a.setAttribute("value", controller.axes[i]);
		}
	}
	/*  console.log(keys, prevKeys)*/
	prevKeys = keys;
	requestAnimationFrame(updateStatus);
}

function DebugPrint(i){
	console.log("current touch :", i)
}

function handleEvent(i, curr, prev) {
	//console.log(i)
	if(i == 7) { // On avance --> Gamepad R2
		if(curr.includes(i) && !prev.includes(i)) {
			console.log("down")
			pepper.ALMotion.move(1, 0, 0);
		} else if(!curr.includes(i) && prev.includes(i)) {
			console.log("up")
			pepper.ALMotion.stopMove();
		}
	}

	if(i == 6) { // On recule --> Gamepad L2
		if(curr.includes(i) && !prev.includes(i)) {
			console.log("down")
			pepper.ALMotion.move(-50, 0, 0);
		} else if(!curr.includes(i) && prev.includes(i)) {
			console.log("up")
			pepper.ALMotion.stopMove();
		}
	}

	if(i == 4) { // On va à gauche --> Gamepad arrow right
		if(curr.includes(i) && !prev.includes(i)) {
			console.log("down")
			pepper.ALMotion.move(0, 50, 0);
		} else if(!curr.includes(i) && prev.includes(i)) {
			console.log("up")
			pepper.ALMotion.stopMove();
		}
	}

	if(i == 5) { // On va à droite --> Gamepad arrow right
		if(curr.includes(i) && !prev.includes(i)){
			console.log("down")
			pepper.ALMotion.move(0, -1, 0);
		} else if(!curr.includes(i) && prev.includes(i)) {
			console.log("up")
			pepper.ALMotion.stopMove();
		}
	}


	if(i == 10) { // On fait une rotation à gauche --> Push Left Joystick 
		if(curr.includes(i) && !prev.includes(i)) {
			console.log("down")
			  pepper.ALMotion.move(0, 0, 3.14);
		} else if(!curr.includes(i) && prev.includes(i)) {
		  	console.log("up")
		  	pepper.ALMotion.stopMove();}
		}

		if(i == 11) { // On fait une rotation à droite --> Push Right Joystick 
		if(curr.includes(i) && !prev.includes(i)){
			console.log("down")
		  	pepper.ALMotion.move(0, 0, -3.14);}
		else if(!curr.includes(i) && prev.includes(i)){
		  	console.log("up")
		  	pepper.ALMotion.stopMove();}
	}
}

function connecthandler(e) {
	addgamepad(e.gamepad);
}

function scangamepads() {
	var gamepads = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads() : []);
	for (var i = 0; i < gamepads.length; i++) {
		if (gamepads[i]) {
			if (!(gamepads[i].index in controllers))
				addgamepad(gamepads[i]);
			else
				controllers[gamepads[i].index] = gamepads[i];
		}
	}
}
