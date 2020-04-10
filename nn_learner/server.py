import os
import socket
import sys

import numpy as np

from tree_size_estimator import TreeSizeEstimator

HOST = ''  # specifies that the socket is reachable by any address the machine happens to have

def parse_network_state(network_state_str):
    network_state = np.array(network_state_str.split(',')).reshape(-1, 1)
    return network_state


def encode_results(estimator_result):
    estimator_result_str = ','.join(map(str, estimator_result)).encode()
    return estimator_result_str


def handleClient(connection, estimator):
    # Read data
    while True:
        data = connection.recv(4096)
        if not data:
            break

        print("from connected user: ", data.decode())  # convert from byte to string

        network_state = parse_network_state(data.decode())
        if estimator.check_input(network_state):
            results = estimator.get_best_constraint(network_state)
            connection.send(encode_results(results))
        else:
            connection.send(encode_results(b'error'))

    connection.close()
    os._exit(0)


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

    while True:
        # accept connections from outside
        (connsocket, address) = serversocket.accept()

        print("Connection from: ", address)

        childPid = os.fork()
        # copy this process
        if childPid == 0:
            # if in child process: handle
            handleClient(connsocket, estimator)
        else:
            # close connection in parent
            connsocket.close()

    serversocket.close()

if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('usage: port_num')
        exit(1)
    port = int(sys.argv[1])
    init_server(port)
