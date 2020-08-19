# Simple connection balancer TCP written in Qt

## Overview

BalancerQt proxy connections from clients to servers specified at startup.
Connections are distributed evenly by the number of active connections.

## Example usage

balancerQt -s=127.0.0.1:6482,127.0.0.1:6483,127.0.0.1:6484 -p=5000 -i=0.0.0.0

## Command line options

Options:
  -?, -h, --help                 Displays this help.
  -p, --port <port>              The port on which the balancer server listens.
  -s, --server <servers>  List of servers to connect format
                                 ip:port,ip:port.
  -i, --interface <interface>    The interface on which the Balancer server
                                 listens.
