#!/usr/bin/env python3

import socket
import numpy as np
import json
import time
import random
import turtle

wn = turtle.Screen()
wn.bgcolor("black")
wn.title("Embedded HW3")
wn.setup(1030, 1040)

word = turtle.Turtle()
word.hideturtle()
word.color("white")
word.penup()
word.goto(-510, 500)

ball = turtle.Turtle()
ball.shape("circle")
ball.color("white")
ball.penup()
ball.speed(0)
ball.dy = 0
ball.dx = 0

init_dx = 0
init_dy = 0

count = 0

HOST = '' # Standard loopback interface address
PORT = 12345 # Port to listen on (use ports > 1023)
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
	s.bind((HOST, PORT))
	s.listen()
	word.write("Waiting to connect", move=False, align="left", font=("Arial", 10, "normal"))
	conn, addr = s.accept()
	word.clear()
	word.write("Connected to " + str(addr), move=False, align="left", font=("Arial", 10, "normal"))
	while True:
		data = conn.recv(1024).decode();
		movement = data.split(",")
		
		if count == 0 :
			init_dx = int(movement[0])
			init_dy = int(movement[1])
			count += 1
		else :
			ball.dx = (int(movement[0]) - init_dx)/10
			ball.dy = (int(movement[1]) - init_dy)/30

		print(ball.dx, ball.dy)

		pos_x = ball.xcor() + ball.dx
		pos_y = ball.ycor() + ball.dy

		if pos_x < -500 : 
			pos_x = -500
		elif pos_x > 500 : 
			pos_x = 500

		if pos_y < -500 : 
			pos_y = -500
		elif pos_y > 500 :
			pos_y = 500
		else :
			pos_y = ball.ycor() + ball.dy

		ball.goto(pos_x, pos_y)

