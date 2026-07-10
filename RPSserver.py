import asyncio
from websockets.asyncio.server import serve
import json
import random

class RPSPlayer:
    playerNum = 0
    cardPool = {'status':'randomised' ,'R':0, 'P':0, 'S':0}
    twoPlayerEvent = asyncio.Event()
    cardRandomisationDoneEvent = asyncio.Event()
    victoryDecidedEvent = asyncio.Event()
    finalQueue = asyncio.Queue(1)
    
    @staticmethod
    async def handleCardUpload(websocket, libmessage):
        #add individual recieved cards into communal card pool
        print("adding cards into card pool")
        RPSPlayer.cardPool['R'] += int(libmessage['R'])
        RPSPlayer.cardPool['P'] += int(libmessage['P'])
        RPSPlayer.cardPool['S'] += int(libmessage['S'])
        if RPSPlayer.twoPlayerEvent.is_set():
            print("first player submitted their cards")
            print(RPSPlayer.cardPool)
            RPSPlayer.twoPlayerEvent.clear()
            await RPSPlayer.cardRandomisationDoneEvent.wait()
            await websocket.send(json.dumps(RPSPlayer.cardPool))
        else:
            print("second player submitted their cards")
            print(RPSPlayer.cardPool)
            privateBuffer = {'status':'randomised','R':0, 'P':0, 'S':0}
            potentialChoices = ['R', 'P', 'S']
            privateBufferCounter = 0
            while privateBufferCounter<3:
                cardType = random.choice(potentialChoices)
                print(cardType)
                if RPSPlayer.cardPool[cardType] > 0:
                    RPSPlayer.cardPool[cardType] -= 1
                    privateBuffer[cardType] += 1
                    privateBufferCounter +=1
                    print(privateBuffer)
                else:
                    potentialChoices.remove(cardType)
            await websocket.send(json.dumps(privateBuffer))
            RPSPlayer.cardRandomisationDoneEvent.set()
    
    @staticmethod
    async def handleShowdown(websocket, libmessage):
        keyFrameMap = {
        'P': ['R', 'P', 'S'],
        'S': ['P', 'S', 'R'],
        'R': ['S', 'R', 'P']
        }
        winnerCalculationArray = [{'outcome':'win'}, {'outcome':'tie'}, {'outcome':'lose'}]
        
        if RPSPlayer.cardRandomisationDoneEvent.is_set():
            RPSPlayer.cardRandomisationDoneEvent.clear()
            print('first player to submit choice into the queue')
            await RPSPlayer.finalQueue.put(libmessage['final'])
            await RPSPlayer.victoryDecidedEvent.wait()
            oppOutcome = await RPSPlayer.finalQueue.get()
            if oppOutcome['outcome'] == 'tie':
                await websocket.send(json.dumps({'status':'final','outcome':'tie'}))
            elif oppOutcome['outcome'] == 'lose':
                await websocket.send(json.dumps({'status':'final','outcome':'win'}))
            else:
                await websocket.send(json.dumps({'status':'final','outcome':'lose'}))
            #game ends
            
        else:
            print('second player to submit choice')
            oppChoice = await RPSPlayer.finalQueue.get()
            myChoice = libmessage['final']
            keyFrame = keyFrameMap[myChoice]
            oppIndex = keyFrame.index(oppChoice)
            myOutcome = winnerCalculationArray[oppIndex]
            myOutcome.setdefault('status','final')
            await websocket.send(json.dumps(myOutcome))
            await RPSPlayer.finalQueue.put(myOutcome)
            RPSPlayer.victoryDecidedEvent.set()
            #game ends        
        
    handlerMap = {
        'cardUpload': handleCardUpload,
        'showdown': handleShowdown,
        }
    
    @staticmethod
    async def processClient(websocket):
        print("process client entered")
        async for jsonmessage in websocket:
            libmessage = json.loads(jsonmessage)
            print(libmessage)
            key = libmessage['status']
            await RPSPlayer.handlerMap.get(key)(websocket, libmessage)
    
    @staticmethod
    async def registerPlayer(websocket):
        readyJson = {'status':'ready'}
        if RPSPlayer.playerNum ==2:
            RPSPlayer.twoPlayerEvent.set()
            
        await RPSPlayer.twoPlayerEvent.wait()
        await websocket.send(json.dumps(readyJson))
        await RPSPlayer.processClient(websocket)

async def handler(websocket):
    jsonmessage = await websocket.recv()
    libmessage = json.loads(jsonmessage)
    if libmessage["status"] != "init":
        print("Did not recieve init")
        #placeholder: replace with exception in final 
    else:
        RPSPlayer.playerNum +=1
        if RPSPlayer.playerNum <=2:
            print("Playernum = "+str(RPSPlayer.playerNum))
            await RPSPlayer.registerPlayer(websocket)
        else:
            print("Too many players")  
                 
async def main():
    async with serve(handler,"",8001) as server:
        await server.serve_forever()
        
if __name__ =="__main__":
    asyncio.run(main())