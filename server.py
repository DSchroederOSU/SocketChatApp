import sys
from socket import *


# main chat function that loops sending and receiving
def beginChat(conn, client, handle):
	message = ""
	while 1:
		# we want to receive up to 500 chars
		receive = conn.recv(501)[0:-1]
		if receive == "":
			print "End of connection"
			print "Waiting..."
			break
		# print client message with appended handle
		print client + "> " + receive
		sending = ""
		while len(sending) > 500 or len(sending) == 0:
			sending = raw_input("{}> ".format(handle))

		# check for stopping condition
		if sending == "\quit":
			print "Connection ended.\nWaiting...\n"
			break
		conn.send(sending)

def createSocket(port):
	my_socket = socket(AF_INET, SOCK_STREAM)
	my_socket.bind(('', port))
	my_socket.listen(1)
	return my_socket

#  this method mirrors the handshake method in the client code
#  and does a simple send and recv to send the client our user name
#  and get the client's user name back
def establishTCP(conn, handle):
	clientHandle = conn.recv(500)
	conn.sendall(handle)
	return clientHandle

# main method that starts the socket server and awaits connections
if __name__ == "__main__":
	# check command args
	if len(sys.argv) != 2:
		print "You must define a port number: chatserve.py [PORT]"
		exit(1)

	my_port = int(sys.argv[1])
	my_socket = createSocket(my_port)

	handle = ""

	while len(handle) > 10 or len(handle) == 0:
		handle = raw_input("Enter a handle (0-10 Chars):\n")
		print "Waiting...\n"
	while 1:
		# from: https://docs.python.org/3/library/socket.html
		conn, addr = my_socket.accept()
		print "Connected on address {}".format(addr)
		clientHandle = establishTCP(conn, handle)
		beginChat(conn, clientHandle, handle)
		conn.close()