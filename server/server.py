from aiohttp import web
import socketio

sio = socketio.AsyncServer(cors_allowed_origins='*')
app = web.Application()
sio.attach(app)

clients = []

@sio.event
def connect(sid, environ):
    print('connect ', sid)
    clients.append(sid)
    print(clients)

@sio.event
def disconnect(sid):
    print('disconnect ', sid)
    clients.remove(sid)
    print(clients)

@sio.event
async def midi(sid, note, velocity):
    if sid in clients:
        clients.remove(sid)
    print(clients)

    print('midi ', note, ' ', velocity)

    colors = {
        100: 0xFF0000, #red
        101: 0x00FF00, #green
        102: 0x0000FF, #blue
        103: 0x00FFFF, #cyan
        104: 0xFF00FF, #magenta
        107: 0xFFFF00, #yellow
        106: 0xFFA500, #orange
        108: 0xFFFFFF, #white
    }
    notes = {
        48: 0,
        50: 1,
        52: 2,
        53: 3,
        55: 4,
        57: 5,
        59: 6,
        60: 7,
        62: 8,
        64: 9,
        65: 10,
        67: 11,
        69: 12,
        71: 13,
        72: 14,
        74: 15,
        76: 16,
        77: 17,
        79: 18,
        81: 19,
        83: 20,
        84: 21,
        86: 22,
        88: 23,
        89: 24,
        91: 25,
        93: 26,
        95: 27,
        96: 28,
    }

    i = notes.get(note)
    if i < len(clients):
        await sio.emit('color', colors.get(velocity, 0xFF00FF), clients[i])

web.run_app(app)
