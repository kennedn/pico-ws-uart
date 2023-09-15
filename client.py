import asyncio
import websockets

# Create a global WebSocket connection variable
websocket = None

async def connect(uri):
    global websocket
    websocket = await websockets.connect(uri)

async def send_message(message):
    if websocket:
        await websocket.send_binary(message)
        print(f"Sent: {message}")
    else:
        print("WebSocket connection not established. Call 'connect()' first.")

async def receive_message():
    if websocket:
        response = await websocket.recv()
        print(f"Received: {response}")
    else:
        print("WebSocket connection not established. Call 'connect()' first.")

async def disconnect():
    global websocket
    if websocket:
        await websocket.close()
        websocket = None
        print("WebSocket connection closed.")
    else:
        print("WebSocket connection not established. Call 'connect()' first.")

if __name__ == "__main__":
    print("WebSocket functions are ready. Call 'connect()' to establish a connection.")
    el = asyncio.get_event_loop()
    a = el.run_until_complete

