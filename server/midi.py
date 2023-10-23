import mido
import socketio

sio = socketio.Client()

@sio.event
def connect():
    print('connection established')

@sio.event
def disconnect():
    print('disconnected from server')

sio.connect('http://localhost:8080')

with mido.open_input() as inport:
    for msg in inport:
        print(msg)
        if msg.type == 'note_on':
            if msg.velocity > 0:
                 sio.emit('midi', (msg.note, msg.velocity))
