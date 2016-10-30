//Copyright (C) 2016 <>< Charles Lohr, see LICENSE file for more info.
//
//This particular file may be licensed under the MIT/x11, New BSD or ColorChord Licenses.


globalParams = {};

function mainticker()
{
/*	KickOscilloscope();
	KickDFT();
	KickNotes();
	KickLEDs();
	QueueOperation( "CVR", ReceiveParameters );*/

	TickLEDs();
	TickParameters();
	setTimeout( mainticker, 1000 );
}

function maininit()
{
	setTimeout( mainticker, 1000 );
}

window.addEventListener("load", maininit, false);

function UpdateValids()
{
}





//////////////////////////////////////////////////////////////////////////////////////////////

function TickParameters()
{
	//We have to update parameters every time because other things rely on them.
	//if( IsTabOpen('Parameters') )
	{
		QueueOperation( "CVR", ReceiveParameters );
	}
}

function ChangeParam( p )
{
	var elem = p.id.substr( 5 );
	QueueOperation( "CVW" + elem + "\t" + encodeURIComponent(p.value) );
}

var hasCreateParams = false;
function ReceiveParameters(req,data) {
	var elems = data.split( "\n" );

	for( var v = 0; v < elems.length; v++ )
	{
		var pair = elems[v].split( "\t" );
		if( pair.length == 2 )
		{
			//console.log( pair[1] + " " + decodeURIComponent(pair[1]) );
			globalParams[pair[0]] = decodeURIComponent( pair[1] );
		}
	}

	if( !hasCreateParams )
	{
		hasCreateParams = true;
		var htv = "<table border=1><tr><th>Value</th><th width=100%>Parameter</th></tr>";

		for( var v in globalParams )
		{
			var vp = globalParams[v];
			htv += "<tr><td><INPUT TYPE=TEXT ID=param" + v + " VALUE=" + vp + " onchange=ChangeParam(this)></td><td>" + v + "</td></tr>";
		}

		$("#InnerParameters").html( htv + "</table>" );

/*
		for( var v in globalParams )
		{
			if( v.substr(0,1) == 'r' )
			{
				$("#param" + v).prop( "disabled", true );
			}
		}
*/		

	}

	for( var v in globalParams )
	{
		var vp = globalParams[v];
		var p = $("#param"+v);
		if( !p.is(":focus" ) )
			p.val(vp);
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////


function TickLEDs()
{
	if( IsTabOpen('LEDs') )
	{
		QueueOperation( "L0\t0\t" + globalParams["top_leds"], ReceiveLEDs );
	}
}


function ReceiveLEDs(req,data) {
	var elems = data.split( "\t" );
	var il = elems[0].substr(1);
	if( il == 0 ) QueueOperation( "L1\t0\t" + globalParams["right_leds"], ReceiveLEDs );
	if( il == 1 ) QueueOperation( "L2\t0\t" + globalParams["bottom_leds"], ReceiveLEDs );
	if( il == 2 ) QueueOperation( "L3\t0\t" + globalParams["left_leds"], ReceiveLEDs );
	var led_start = Number( elems[1] );
	var led_count = Number( elems[2] );

	var ls = document.getElementById('LEDCanvasHolder');
	var canvas = document.getElementById('LEDCanvas');
	var ctx = canvas.getContext('2d');
	var h = ls.height;
	var w = ls.width;


	if( canvas.width != ls.clientWidth-10 )   canvas.width = ls.clientWidth-10;
	if( ctx.canvas.width != canvas.clientWidth )   ctx.canvas.width = canvas.clientWidth;

	var width = canvas.width;
	var height = canvas.height;

	if( il == 0 ) {
		ctx.fillStyle = "#000000";
		ctx.lineWidth = 0;
		ctx.fillRect( 0, 0, width, height );
	}

	var startx = 0;
	var starty = 0;
	var xdir = 0;
	var ydir = 0;

	if( il == 0 ) { startx = 10; starty = 10; xdir = (width-20)/led_count; ydir = 0; }
	if( il == 1 ) { startx = ctx.canvas.width-10; starty = 10; xdir = 0; ydir = (height-20)/led_count; }
	if( il == 2 ) { startx = 10; starty = ctx.canvas.height - 10; xdir = (width-20)/led_count; ydir = 0; }
	if( il == 3 ) { startx = 10; starty = 10; xdir = 0; ydir = (height-20)/led_count; }

	var x = startx + xdir/2;
	var y = starty + ydir/2;

	for( i = 0; i < led_count; i++ )
	{
		samp = elems[3].substr( i*6, 6 );

		ctx.beginPath();
		ctx.arc( x, y, 3, 0, 2 * Math.PI, false);
		ctx.fillStyle = "#" + samp.substr( 0, 2 ) + samp.substr( 2, 2 ) + samp.substr( 4, 2 );
		ctx.fill();
		ctx.lineWidth = .5;
		ctx.strokeStyle = "#ffffff";
		ctx.stroke();

		x += xdir;
		y += ydir;
	}


//	console.log( data );
	
/*
	var samps = Number( secs[1] );
	var data = secs[2];
	var lastsamp = parseInt( data.substr(0,4),16 );
	ctx.clearRect( 0, 0, canvas.width, canvas.height );

	for( var i = 0; i < samps; i++ )
	{
		var x2 = i * canvas.clientWidth / samps;
		var samp = data.substr(i*6,6);
		var y2 = ( 1.-samp / 2047 ) * canvas.clientHeight;

		ctx.fillStyle = "#" + samp.substr( 2, 2 ) + samp.substr( 0, 2 ) + samp.substr( 4, 2 );
		ctx.lineWidth = 0;
		ctx.fillRect( x2, 0, canvas.clientWidth / samps+1, canvas.clientHeight );
	}

	var samp = parseInt( data.substr(i*2,2),16 );
*/
} 



