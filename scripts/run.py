#!/usr/bin/env python

import asyncio
import json
import websockets

request_id = 0

def create_request(method, params=dict()):
    global request_id
    request_id = request_id + 1
    return json.dumps(
        dict(
            jsonrpc="2.0",
            id=request_id,
            method=method,
            params=params))

async def processor(uri):
    async with websockets.connect(uri, ping_interval=30) as websocket:
        print('connected')
        await websocket.send(create_request("public/get_currencies"))
        await websocket.send(create_request("public/get_instruments", dict(currency="BTC", kind="future")))
        async for message in websocket:
            print(f'{message}')

asyncio.get_event_loop().run_until_complete(
    processor('wss://test.deribit.com/ws/api/v2'))
