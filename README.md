# SocketChatApp
Implement a client-server network application for CS 372 at Oregon State University


### Step 1:
Begin the python chat server by running:

```python chatserve.py [PORT]```

This will begin the chat server and prompt you for a user handle.

The server will then be ready and waiting to accept connections.


### Step 2:
In a new terminal window, start up the chat client by running:

```make```

and then run the executable:

```./chatclient 0.0.0.0 [PORT]```

The client program will attempt to connect to the server at the given address and port.

The client will be prompted for a user handle.

### Step 3:

Begin chatting! 

The client will initiate the first message in one console window, then the server will receive the message and be able to reply in the other console window.