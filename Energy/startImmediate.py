#getlast
import sys
import telnetlib
PROMPT = "A>"

def init(h,p):
	HOST = h 
	PORT = p
	tn = telnetlib.Telnet(HOST, PORT)
	tn.open(HOST, PORT)
	tn.read_until(PROMPT)
	reading = read(tn)
	while (reading > 0):
		trySetBack(tn)
		tn.read_until(PROMPT,2)
		reading = int(read(tn))
	tn.write("INIT\r\n")
	#tn.read_eager()
	tn.close()


#Read points
def read(tn):
	tn.write("DATA:POINTS?\r\n")
	res = tn.read_until("\n", 2)
	tn.read_until(PROMPT,1)
	return res.strip()[1:]

def trySetBack(tn):
	tn.write("ABORT\r\n")
	tn.read_until(PROMPT)
	points = read(tn)
	if (points > 0):
		rs = "DATA:REMOVE? " + points + "\r\n"
		tn.write(rs)
	tn.read_until(PROMPT)
	

#init()
