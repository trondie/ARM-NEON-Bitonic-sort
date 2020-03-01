#getlast
import sys
import telnetlib

PROMPT = "A>"

def stop(h,p):
	HOST = h
	PORT = p
	tn = telnetlib.Telnet(HOST, PORT)
	tn.open(HOST, PORT)
	tn.read_until(PROMPT)
	tn.write("ABORT\r\n")
	tn.read_until(PROMPT)
	points1 = read(tn)
	points2 = read(tn)
	while (points1 != points2):
		tn.write("ABORT\r\n")
		tn.read_until(PROMPT)
		points1 = read(tn)
		points2 = read(tn)
	#tn.read_eager()
	tn.close()

#Read points
def read(tn):
	tn.write("DATA:POINTS?\r\n")
	res = tn.read_until("\n", 2)
	tn.read_until(PROMPT,1)
	return res.strip()[1:]

#stop()
