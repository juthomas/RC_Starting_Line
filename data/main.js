
function sleep(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
  }

async function startSound() {
	var audio;
	// await sleep(1000);
	audio = new Audio('Race_Start.mp3');
	audio.play();

	console.log("Hello");
	// audio = new Audio('3.mp3');
	// audio.play();
	// await sleep(1000);
	// audio = new Audio('2.mp3');
	// audio.play();
	// await sleep(1000);
	// audio = new Audio('1.mp3');
	// audio.play();
	// await sleep(1000);
	// audio = new Audio('c_est_parti.mp3');
	// audio.play();
	console.log("End Of sounds");
}


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
	var lapTimeOnBtn = document.getElementById("lapTimeOnBtn");
	var clearBtn = document.getElementById("clearBtn");
	
	var sensorUpdateInput = document.getElementById("sensorUpdateInput");
	var sensorUpdateBtn = document.getElementById("sensorUpdateBtn");
	var sensorRefinementInput = document.getElementById("sensorRefinementInput");
	var sensorRefinementBtn = document.getElementById("sensorRefinementBtn");
	var sensorMaxDistanceInput = document.getElementById("sensorMaxDistanceInput");
	var sensorMaxDistanceBtn = document.getElementById("sensorMaxDistanceBtn");




	var redOn = false;
	var orangeOn = false;
	var greenOn = false;
	var lapTimeOn = true;
	lapTimeOnBtn.style.backgroundColor = "green";


	startBtn.addEventListener('click', () => {
		startSound();
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
	lapTimeOnBtn.addEventListener('click', () => {
		lapTimeOn = !lapTimeOn;
		if (lapTimeOn)
		{
			lapTimeOnBtn.style.backgroundColor = "green";
			Socket.send("[LAPTIME] ON");
		}
		else
		{
			lapTimeOnBtn.style.backgroundColor = "";
			Socket.send("[LAPTIME] OFF");
		}
	})
	clearBtn.addEventListener('click', () => {
		document.getElementById("rxConsole").value = "";
	})

	sensorUpdateBtn.addEventListener('click', () => {
		if (sensorUpdateInput.value)
		{
			Socket.send("[SETSENSORUPDATE] " + sensorUpdateInput.value);
		}
	})
	sensorRefinementBtn.addEventListener('click', () => {
		if (sensorRefinementInput.value)
		{
			Socket.send("[SETREFINEMENT] " + sensorRefinementInput.value);
		}
	})
	sensorMaxDistanceBtn.addEventListener('click', () => {
		if (sensorMaxDistanceInput.value)
		{
			Socket.send("[SETSENSORMAXDISTANCE] " + sensorMaxDistanceInput.value);
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

			if (event.data.startsWith("[LAPSOUND]"))
			{
				var audio;
				// await sleep(1000);
				audio = new Audio('RaceCheckPoint.mp3');
				audio.play();
			}
			else if (event.data.startsWith("[LAPTIME]"))
			{
				document.getElementById("rxConsole").value += (event.data);
				document.getElementById("rxConsole").scrollTop = document.getElementById("rxConsole").scrollHeight;
				var audio;
				// await sleep(1000);
				audio = new Audio('RaceCheckPoint.mp3');
				audio.play();
			}
			else if (event.data.startsWith("[RACETIME]"))
			{
				document.getElementById("rxConsole").value += (event.data);
				document.getElementById("rxConsole").scrollTop = document.getElementById("rxConsole").scrollHeight;
				var audio;
				// await sleep(1000);
				audio = new Audio('Applause.mp3');
				audio.play();
			}

			// document.getElementById("rxConsole").value += event.data;
			// document.getElementById("rxConsole").scrollTop = document.getElementById("rxConsole").scrollHeight;
		});
	}
})
