from telegram.ext import Updater, InlineQueryHandler, CommandHandler, MessageHandler, Filters
import paho.mqtt.client as mqtt
from config import BotToken, BrokerHostname, AuthUsers, QueryParams
from flask import Flask, request

app = Flask(__name__)

# TODO rename this to "engine" or something like that.
# TODO freeze requirements so no breaking changes occur.

# /!\ WARNING: THIS METHOD IS REALLY UNSAFE AND
# SHOULD AT LEAST BE RUN OVER HTTPS.
# TODO FIX this
@app.route("/home_gate/toggle", methods=['GET'])
def route_toggle_gate():
    _username = request.args.get('username', '')
    _password = request.args.get('password', '')
    if _username == QueryParams[0] and password == QueryParams[1]:
        mqtt_toggle_gate()
        return "Success"
    else:
        return "Fail"

def telegram_toggle_gate(update, context):
    #check for authorized users...
    if not update.message.from_user.username in AuthUsers:
        context.bot.send_message(chat_id=update.effective_chat.id, text="oops, you ain't authorized fella!")
        return
    context.bot.send_message(chat_id=update.effective_chat.id, text="Portãozinho ta mexenu...")
    mqtt_toggle_gate()

def mqtt_toggle_gate():
    mq_client.publish('/topic/gate', payload='GATE', qos=0, retain=False)

def start(update, context):
    context.bot.send_message(chat_id=update.effective_chat.id, text="Controle da casa. Veja bem o q ce vai fazer.")

if __name__ == '__main__':

    #mqtt client
    global mq_client
    mq_client = mqtt.Client()
    mq_client.connect(BrokerHostname, 1883, 60)

    #telegram bot
    updater = Updater(token=BotToken, use_context=True)
    dp = updater.dispatcher
    dp.add_handler(CommandHandler('start', start))
    dp.add_handler(CommandHandler('gate', telegram_toggle_gate))

    updater.start_polling()
    updater.idle()
