from telegram.ext import Updater, InlineQueryHandler, CommandHandler, MessageHandler, Filters
import paho.mqtt.client as mqtt
from config import BotToken, BrokerHostname, AuthUsers

def toggle_gate(update, context):
    #check for authorized users...
    if not update.message.from_user.username in AuthUsers:
        context.bot.send_message(chat_id=update.effective_chat.id, text="oops, you ain't authorized fella!")
        return

    context.bot.send_message(chat_id=update.effective_chat.id, text="Portãozinho ta mexenu...")
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
    dp.add_handler(CommandHandler('gate', toggle_gate))

    updater.start_polling()
    updater.idle()
