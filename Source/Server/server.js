// Imports 
import WebSocket, { WebSocketServer } from 'ws';
import { randomUUID } from 'crypto';

// Variables
const clients = new Map(); // has to be a Map instead of {} due to non-string keys
const players = new Map(); // Each id will have its own player class instantiated
const wss = new WebSocketServer({ port: 8080 }); // initiate a new server that listens on port 8080

// Player Class
class Player{
    constructor(id, ws) {
        this.id = id;
        this.ws = ws;
        this.name = `Player_${id.slice(0,4)}`;
        this.position = { x: 0, y: 0, z: 0};
        this.rotation = { pitch: 0, yaw: 0, roll: 0 };
    }
}

// Initial Boot Print
console.log('Booting Server...');

// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------------------------------------------


// Function for sending a message to every connected client
function serverBroadcast(message) {
    wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(message);
        }
    });
}

// Function for sending broadcast to everyone EXCEPT one client
function broadcastExcept(excludeWs, data) {
    const json = JSON.stringify(data);
    wss.clients.forEach((client) => {
        if (client !== excludeWs && client.readyState === WebSocket.OPEN) {
            client.send(json);
        }
    })
}

// Function for sending a message to everyone
// This is a secondary version of the above function.
function broadcast(data){
    broadcastExcept(null, data);
}

// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------------------------------------------

// Event Handling

// Set up event handlers and do other things upon a client connecting to the server
wss.on('connection', (ws) => {
    // Create an id to track the client
    const id = randomUUID();
    const player = new Player(id, ws);
    
    clients.set(ws, id);
    players.set(ws, player);
    
    // Tell CURRENT player their ID + state
    ws.send(JSON.stringify({
        type: "welcome",
        assignedId: id,
        message: "Welcome!"
    }));
    console.log(`new connection assigned id: ${id}`);

    // Tell CURRENT player aboutall other players
    players.forEach((player, playerws) => {
        if(playerws !== ws){
            ws.send(JSON.stringify({
                type: "playerJoined",
                id: player.id,
                name: player.name,
                position: player.position,
                rotation: player.rotation
            }));
        }
    });

    // Tell EVERYONE ELSE that a new player has joined
    broadcastExcept(ws, {
        type: "playerJoined",
        id: player.id,
        name: player.name,
        position: player.position,
        rotation: player.rotation
    });

    // Handling incoming messages
    ws.on('message', (data) => {

        const raw = data.toString().trim();
        //console.log('Raw incoming:', raw);   

        let message;
        try {
            message = JSON.parse(raw);
        } catch (e) {
            console.error("JSON parse failed:", e.message);
            console.error("Failed raw data:", raw);
            return;  // skip processing any bad messages
        }

        //const message = JSON.parse(data);

        if (message.type === "move") {
            player.position = message.position;
            player.rotation = message.rotation || player.rotation;

            // Send this movement to all other clients
            broadcastExcept(ws, {
                type: "playerMoved",
                id: player.id,
                position: player.position,
                rotation: player.rotation
            });
        }

        if (message.type == "fire") {
            broadcastExcept(ws, {
                type: "playerFired",
                id: player.id,
                hitLocation: message.hitLocation,
            });

            console.log(`Player ${player.id.slice(0, 8)} fired → hit at ${JSON.stringify(message.hitLocation)}`);
        }

        //console.log(`received: ${data}`);
        //serverBroadcast(`Client ${clients.get(ws)} ${data}`);
    });

    // Stop tracking the client upon that client closing the connection
    ws.on('close', () => {
        console.log(`connection (id = ${clients.get(ws)}) closed`);
        players.delete(ws);
        clients.delete(ws);

        broadcast({
            type: "playerLeft",
            id: player.id
        });
    });
});

// ---------------------------------------------------------
// ---------------------------------------------------------
// ---------------------------------------------------------

// Send a message to all the connected clients about how many of them there are every 15 seconds
setInterval(() => {
    console.log(`Number of connected clients: ${clients.size}`);
    console.log(`Number of active players: ${players.size}`);
    serverBroadcast(`Number of connected clients: ${clients.size}`);
}, 15000);


console.log('The server is now running...');
setInterval(() => {
    console.log('...');
}, 30000);


/*
Task Breakdown:

Create a player state when a player connects
Each player will have its own transform rotation
Add another array, once we have the client now create a player for them
Each player will have its own class - all it will need is the transform

When creates - broadcast meessage to all clients to say a player has been created
When a new connects - check is there other players connected for that player

When a client presses "forward" (ie) 
send message data to say player has changed trasnform
then apply to each client

Review CLient Side Prediction to account for latency
Server Reconcilision
*/
