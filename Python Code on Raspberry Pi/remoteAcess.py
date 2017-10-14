import time
import serial
import bluetooth

from flask import Flask, render_template

app = Flask(__name__)

port=serial.Serial("/dev/ttyAMA0",9600,timeout=3.0)
temp,humidity,soil,light,distance= bluetooth.abc()

@app.route('/')
def hello_world():
	return render_template('index.html')    #index.html improove
		
@app.route('/temphum')
def temphum():
	templateData = {
		'temperature':temp,
		'humidity':humidity
	}
	return render_template('temphum.html',**templateData)
	
@app.route('/fanon')
def fanon():
	port.write('1')   			#sending 1 to turn fans on on the IoT kit
	return render_template('fanoff.html')
	
@app.route('/fanoff')
def fanoff():
	port.write('2')  			#sending 2 to stop the fan
	return render_template('fanofflast.html')
	
@app.route('/waterlevel')
def waterlevel():
	templateData = {
		'distance':distance
	}
	return render_template('checkwaterlevel.html',**templateData)

@app.route('/waterplants')
def waterplants():
	templateData = {
		'soilmoisture':soil,
		'temperature':temp,
		'lightintensity':light
		'humidity':humidity
	}
	return render_template('status.html',**templateData)

@app.route('/motoron')
def motoron():
	port.write('3')				#sending 3 to turn on motor
	return render_template('motoron.html')

@app.route('/motoroff')
def motoroff():
	port.write('4')     			#to turn off the motor
	return render_template('motoroff.html')

@app.route('/lights')
def lights():
	port.write('5')   			#to turn on the lights
	return render_template('ledon.html')
	
@app.route('/ledoff')
def ledoff():
	port.write('6')    			#to turn off the led
	return render_template('ledoff.html')
	

if __name__=="__main__":
	app.run(debug=True, host='0.0.0.0')
