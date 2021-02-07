import os
import paho.mqtt.client as mqtt

def suspend_pc():
    os.system("systemctl suspend")

def on_connect_cb(client, userdata, msg, rc):
    client.subscribe('/topic/pc')

def on_message_cb(client, userdata, msg):
    pld = msg.payload.decode('utf-8')
    print('kappa1' + pld)
    if pld == 'suspend':
        print('kappa')
        suspend_pc()

if __name__ == '__main__':

    mq_client = mqtt.Client()
    mq_client.on_message = on_message_cb #On received message
    mq_client.on_connect = on_connect_cb
    #should put on subscribe and so on later.

    mq_client.connect('192.168.100.4', 1883, 60)
    mq_client.loop_forever()
