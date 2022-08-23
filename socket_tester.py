import socket

HOST = "127.0.0.1"
PORT = 8888

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    while True:
        s.sendall(bytes(input('> '), 'ascii'))