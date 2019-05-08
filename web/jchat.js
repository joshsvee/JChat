var output = null;
var input  = null;

window.onload = initialize;

function initialize()
{
	var test = null;
	 
	 if(test != null && test.length > 5)
	 {
		 alert("Da Fuck");
	 }
		
	input = document.getElementById('input');
	input.onkeypress = onEnter;
	output = document.getElementById('output');
	writeOutput("Connecting to server...");
	
	var socket = new WebSocket("ws://li16-110.members.linode.com:881/");
	socket.onclose = onClose;
	socket.onopen = onOpen;
	socket.onmessage = onMessage;
}

function onEnter(e)
{
	var evt = e || window.event;
	if(evt.keyCode == 13 && evt.charCode == 0)
	{
		input.value = "";
		
		return false;
	}
}

function writeOutput(text)
{
	output.innerHTML += text;
}

function onClose(e)
{
	alert("Closed");
}

function onOpen(e)
{
	writeOutput("Connected!" + '<br />');
}

function onMessage(e)
{
	alert("got: " + e.data);
}
