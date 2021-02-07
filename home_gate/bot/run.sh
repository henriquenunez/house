#!/bin/sh

sudo docker build -t home_automator:v1 .
sudo docker run -d home_automator:v1
