import sys
from socket import *


def chat(connection, client, handle):
	message = ""            #Hold the message
	while 1:                #Run until we no longer want to chat
		receive = connection.recv(501)[0:-1]        #Receive message
		if receive == "":                  #If nothing is received. Wait for new connection
			print "End of connection"
			print "Waiting..."
			break

		print "{}> {}".format(client, receive)     #Puts the prompt in the propert format
		sending = ""
		while len(sending) > 500 or len(sending) == 0:
			sending = raw_input("{}> ".format(handle))

		if sending == "\quit":              #Per assignment specs, must be able to quit when you type in \quit
			print "End of connection"
			print "Waiting..."
			break
		connection.send(sending)

# this function does a simple send and recv to send the client our user name
# and get the client's user name back
def exchangeInfo(connection, handle):
	clientName = connection.recv(1024)
	connection.send(handle)
	return clientName

if __name__ == "__main__":
	if len(sys.argv) != 2:              #First check if the user put in the right number of arguments. If not, print the usage statement
		print "Usage: python chatServer.py [port]"
		exit(1)

	portNumber = sys.argv[1]
	newSocket = socket(AF_INET, SOCK_STREAM)         #Taken from here https://docs.python.org/2/howto/sockets.html

	newSocket.bind(('', int(portNumber)))           #Also from this link https://docs.python.org/2/howto/sockets.html

	newSocket.listen(1)         #Start listening

	handle = ""
	while len(handle) > 10 or len(handle) == 0:     #Get the user name, estabish connection, call chat function and finally close when we're ready
		handle = raw_input("Enter a username that is 10 characters or less. ")
		print "Server is  ready for messages."
	while 1:
		connection, address = newSocket.accept()
		print "Connected on address {}".format(address)
		clientName = connection.recv(1024)
		connection.send(handle)

		chat(connection, clientName, handle)

		connection.close()