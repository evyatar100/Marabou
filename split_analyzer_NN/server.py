import os
import socket
import sys
import threading


import numpy as np

from tree_size_estimator import TreeSizeEstimator

BUF_SIZE = 32768

HOST = ''  # specifies that the socket is reachable by any address the machine happens to have

def parse_network_state(network_state_str):
    network_state = np.array(network_state_str.split(',')).reshape(1, -1).astype(np.float32)
    return network_state


def encode_results(estimator_result):
    estimator_result_str = ','.join(map(str, estimator_result)).encode()
    return estimator_result_str


def handleClient(connection, estimator, thread_name):

    prefix = thread_name + ':'

    # Read data
    while True:
        data = connection.recv(BUF_SIZE)
        if not data:
            break

        print(prefix, 'from connected user: ', data.decode())  # convert from byte to string

        network_state = parse_network_state(data.decode())
        if estimator.check_input(network_state):
            print(prefix, 'input is proper, waiting for estimation...')
            results = estimator.get_best_constraint(network_state)
            results_str = encode_results(results)
            print(prefix, 'got result from model.')
            connection.send(results_str)
            print(prefix, 'result was sent to the client.')
        else:
            connection.send(b'error')

    connection.close()


def init_server(port):

    # create an INET, TCP socket
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # bind the socket to a public host, and a well-known port
    serversocket.bind((HOST, port))

    # become a server socket
    serversocket.listen(2)

    print("loading Model...")
    estimator = TreeSizeEstimator()
    estimator.restore_model()
    print("Model Loaded.")

    print("Server Listening...")

    i = 1
    while True:
        # accept connections from outside
        (connsocket, address) = serversocket.accept()
        print("Connection from: ", address)

        thread_name = f'thread {i}'
        thread = threading.Thread(target=handleClient, args=(connsocket, estimator, thread_name))
        thread.start()

        # handleClient(connsocket, estimator)
        i += 1

    serversocket.close()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('usage: port_num')
        exit(1)
    port = int(sys.argv[1])
    print(f'port = {port}')
    init_server(port)
