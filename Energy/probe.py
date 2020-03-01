#getlast
import sys
import telnetlib
import math

#Mean input voltage 
DC = 5.0

#Shunt resistance
SHUNT_R = 0.012

#Connection information etc to Agilent multimeter
HOST = "129.241.111.212"
PORT = "5024"
PROMPT = "A>"

#Number of digits in numbers retrieved - Based on the fixed format of samples logged
PRECISION = 8+2
SAMPLES_SEC = 1000.0

#Initiate telnet connection to Agilent multimeter
def initiate():
	tn = telnetlib.Telnet(HOST, PORT)
	tn.open(HOST, PORT)
	return tn

#Telnet to Agilent to read samples
def read(tn):
	tn.read_until(PROMPT)
	tn.write("FETCh?\r\n")
	res = tn.read_until(PROMPT)
	return res

#Simplify input and return voltage vector - Based on the format of the samples logged!
def simplify(st): 
	pos = 0
	num = " "
	vec = []
	n = 0
	for i in range(len(st)):
		if st[i] == '+':
			d = i+1
			while (st[d] != 'E'):
				num += st[d]
				pos+=1
				d+=1
			#i += d
			numf = float(num)
			numf = numf * (pow(10,-int(st[d+3])))
			vec.append(numf)
			num = ""
			numf = 0
			n+=1
	return vec

#Calculate current vector
#No cutoff considered
def iVec(uVec):
	iVec = []
	current = 0.0
	for i in range(len(uVec)):
		current = float(uVec[i]/SHUNT_R)
		iVec.append(current)
	return iVec

#Calculate power vector
#No cutoff considered
def pVec(icVec):
	pwVec = []
	power = 0.0
	for i in range(len(icVec)):
		power = float(icVec[i]*(DC-(icVec[i]*SHUNT_R)))
		pwVec.append(power)
	return pwVec

#Mean power w.r.t voltage vector
#Cutoff cannot result to be equal to sampletime. This however does never happend.
def meanPowerU(uVec, cutoff):
	pwVec = []
	power = 0.0
	#Get amount of samples to cut off
	nStop = int(SAMPLES_SEC * cutoff)
	for i in range(len(uVec)-nStop):
		power += float(uVec[i]/SHUNT_R) * (DC-uVec[i])
	return (power/(len(uVec)-nStop))

#Consumed Energy w.r.t power vector. Integrate samlpes from power vector
#No cutoff considered
def energy(pwVec):
	E = 0.0
	N = len(pwVec)
	dt = 1.0/SAMPLES_SEC
	for i in range(N-1):
		E += dt * float(((pwVec[i+1]+pwVec[i])/2))
	return E

#Consumed Energy calculated w.r.t voltage vector.
#cutoff is t_c. t_c = t_s - t_k
def energyU(uVec, cutoff):
	E = 0.0
	N = len(uVec)
	dt = 1.0/SAMPLES_SEC
	#Get amount of samples to cut off
	nStop = int(SAMPLES_SEC * cutoff)
	pt = [0.0, 0,0]
	for i in range(N-1-nStop):
		pt[0] = float(uVec[i]/SHUNT_R) * (DC-uVec[i])
		pt[1] = float(uVec[i+1]/SHUNT_R) * (DC-uVec[i])
		E += dt * float(((pt[1]+pt[0])/2))
	return E

#Mean power
def meanPower(iVec):
	acp = 0.0
	for i in range(len(iVec)):
		acp += (iVec[i]*DC)
	return (acp/len(iVec))

#Mean power called externally
def meanPowerExtern(kernelTime):
	tn = initiate()
	reading = read(tn)
	tn.close()
	#Voltage vector
	uVec = []
	uVec = simplify(reading)
	#Power vector
	power = 0.0
	cutoff = calculateCutoff(kernelTime)
	power = meanPowerU(uVec, cutoff)
	return power

#Consumed energy called externally
def energyExtern(kernelTime):
	tn = initiate()
	reading = read(tn)
	tn.close()
	#Voltage vector
	uVec = []
	uVec = simplify(reading)
	cutoff = calculateCutoff(kernelTime)
	#if (t1-cutoff < 0.0):
	#	cutoff = 0.0
	E = energyU(uVec, cutoff)
	return E

#Time duration called externally with cutoff
def timeDurationCutoffExtern(cutoff):
	tn = initiate()
	reading = read(tn)
	tn.close()
	uVec = []
	uVec = simplify(reading)
	nStop = int(SAMPLES_SEC * cutoff)
	#if ((len(uVec)-nStop) < 0):
	#	nStop = 0
	return float(((len(uVec)-nStop)/SAMPLES_SEC))

#Time duration of logging with cutoff
def timeDurationCutoff(vec, cutoff):
	nStop = int(SAMPLES_SEC * cutoff)
	return float(((len(vec)-nStop)/SAMPLES_SEC))

#Time duration without cutoff
def timeDuration(vec):
	return float(len(vec)/SAMPLES_SEC)

#Calculates the cutoff value, t_c
#This value is used with the interval (0, (ts-tc)) for calculations
def calculateCutoff(kernelTime):
	tn = initiate()
	tn.read_until(PROMPT)
	points = int(countPoints(tn))
	tn.close()
	sampleTime = float(points/SAMPLES_SEC)
	cutoff = sampleTime - kernelTime
	if (cutoff < 0.0):
		return 0.0
	return cutoff

#Read points
def countPoints(tn):
	tn.write("DATA:POINTS?\r\n")
	res = tn.read_until("\n", 2)
	tn.read_until(PROMPT,1)
	return res.strip()[1:]


############################
#Initiate connection and read data
'''
tn = initiate()
reading = read(tn)
tn.close()

print "Time duration with cutoff :", timeDurationCutoffExtern(0.217)
print "Time duration python : ", timeDurationCutoffExtern(0.0)
print "ENERGY : ", energyExtern(0.1);'''
#text_file = open("Output.txt", "w")
#text_file.write("Readings: %s"%reading)
#text_file.close()

#streng = "En to tre fire fem seks"
#for i in range(len(streng)):
#	print streng[i]
#print reading

#Voltage vector
#uVec = []
#uVec = simplify(reading)
#Current vector
#icVec = []
#icVec = iVec(uVec)
#Power vector
#pwVec = []
#pwVec = pVec(icVec)
#print pwVec
#Accumulated energy
#E = 0.0
#E = energy(pwVec)
#print "Diff : ", energyExtern()-E

#power = meanPower(icVec)
#print ("MEAN POWER : "), meanPowerExtern()
#print ("TIME : "), timeDuration(pwVec)
#print ("ACCUMULATED ENERGY: "), energyExtern() 
#print ("SAMPLES : "), len(uVec)
#print uVec
#print icVec

#Telnet to Agilent to read samples
