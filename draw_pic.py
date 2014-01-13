#! /usr/bin/python

import os

wid = 1680
height = 1050

c_list = [[0.285,-0.01],[-0.8, 0.156],[0.5, -0.20],[0.285, 0],[-0.8, 0.156]]

i = 0
for i in range(len(c_list)):#range(len(c_list)):
	os.system("./julia %d %d %f %f %d.bmp &"%(wid,height,c_list[i][0],c_list[i][1],i))
	#os.system("eog 2.bmp &")
