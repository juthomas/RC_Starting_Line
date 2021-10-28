

document.addEventListener("DOMContentLoaded", function (event) {
	// try
	// {
		var Socket = null;
		if (window.location.hostname != "")
			Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
		// var Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
	// }
	// catch (exception)
	// {
	// //   print(exception);
	// }

	var startBtn = document.getElementById("startBtn");
	var stopBtn = document.getElementById("stopBtn");
	var offBtn = document.getElementById("offBtn");
	var redBtn = document.getElementById("redBtn");
	var orangeBtn = document.getElementById("orangeBtn");
	var greenBtn = document.getElementById("greenBtn");

	var redOn = false;
	var orangeOn = false;
	var greenOn = false;


	startBtn.addEventListener('click', () => {
		Socket.send("[START]");
	})
	stopBtn.addEventListener('click', () => {
		Socket.send("[STOP]");
	})
	offBtn.addEventListener('click', () => {
		Socket.send("[OFF]");
	})

	redBtn.addEventListener('click', () => {
		redOn = !redOn;
		if (redOn)
		{
			redBtn.style.backgroundColor = "red";
			Socket.send("[RED] ON");
		}
		else
		{
			redBtn.style.backgroundColor = "";
			Socket.send("[RED] OFF");
		}
	})
	orangeBtn.addEventListener('click', () => {
		orangeOn = !orangeOn;
		if (orangeOn)
		{
			orangeBtn.style.backgroundColor = "orange";
			Socket.send("[ORANGE] ON");
		}
		else
		{
			orangeBtn.style.backgroundColor = "";
			Socket.send("[ORANGE] OFF");
		}
	})
	greenBtn.addEventListener('click', () => {
		greenOn = !greenOn;
		if (greenOn)
		{
			greenBtn.style.backgroundColor = "green";
			Socket.send("[GREEN] ON");
		}
		else
		{
			greenBtn.style.backgroundColor = "";
			Socket.send("[GREEN] OFF");
		}
	})


	// setInterval(() => {
	// 	if (Socket)
	// 		Socket.send("test");
	// }, 1000);
	if (Socket)
	{

		Socket.addEventListener('message', function (event) {
			// console.log('Message from server ', event.data);
			document.getElementById("rxConsole").value += event.data;
			document.getElementById("rxConsole").scrollTop = document.getElementById("rxConsole").scrollHeight;
		});
	}
})
