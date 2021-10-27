document.addEventListener("DOMContentLoaded", function (event) {
	try
	{
	  Socket = new WebSocket('ws://' + window.location.hostname + ':81/');
	}
	catch (exception)
	{
	  print(exception);
	}

	setInterval(loop, 1000);
	Socket.addEventListener('message', function (event) {
		console.log('Message from server ', event.data);
	});
})


function loop()
{
	Socket.send("test");
}