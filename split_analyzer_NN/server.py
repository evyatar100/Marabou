import os
import socket
import sys
import threading
import argparse

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


def handleClient(connection, estimator, thread_name, is_debug):

    prefix = thread_name + ':'

    # Read data
    while True:
        data = connection.recv(BUF_SIZE)
        if not data:
            break

        if is_debug:
            print(prefix, 'from connected user: ', data.decode())  # convert from byte to string
        else:
            print(prefix, 'input for client')

        network_state = parse_network_state(data.decode())
        if estimator.check_input(network_state):
            if is_debug:
                print(prefix, 'input is proper, waiting for estimation...')
            results = estimator.get_best_constraint(network_state)
            results_str = encode_results(results)
            if is_debug:
                print(prefix, 'got result from model.')
            connection.send(results_str)
            if is_debug:
                print(prefix, 'result was sent to the client.')
        else:
            print('input is not valid, sent \"error\" to client')
            connection.send(b'error')

    connection.close()


def init_server(name_dir, port, is_debug):

    # create an INET, TCP socket
    serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # bind the socket to a public host, and a well-known port
    serversocket.bind((HOST, port))

    # become a server socket
    serversocket.listen(2)

    print("loading Model...")
    estimator = TreeSizeEstimator(10, 100, name_dir)
    estimator.restore_model()
    print("Model Loaded.")

    print("Server Listening...")

    i = 1
    while True:
        # accept connections from outside
        (connsocket, address) = serversocket.accept()
        thread_name = f'thread {i}'

        print(f"Connection from: {address}. associated with {thread_name}.")

        thread = threading.Thread(target=handleClient, args=(connsocket, estimator, thread_name, is_debug))
        thread.start()

        i += 1

    serversocket.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Start the server')
    parser.add_argument('name_dir', type=str, help='name of dir with network info')
    parser.add_argument('port', type=int, help='port for server')
    parser.add_argument('--debug', action='store_true', help='Whether or not to print debug info')
    args = parser.parse_args()

    #
    # if len(sys.argv) != 3:
    #     print('usage: name_dir port_num')
    #     exit(1)
    # name_dir = sys.argv[1]
    # port = int(sys.argv[2])
    name_dir = args.name_dir
    port = args.port
    is_debug = args.debug

    print(f'name = {name_dir}')
    print(f'port = {port}')
    init_server(name_dir, port, is_debug)
