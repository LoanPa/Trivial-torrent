# Trivial Torrent Project: Client and Server

## Trivial Torrent Client

You need to implement the client part of the Trivial Torrent protocol. See Trivial Torrent protocol specification document for details (see section 2.3.1 of the specification document).

Use the file `src/ttorrent.c`.

The `ttorrent` command shall work as follows:

~~~{.diff}
$ bin/ttorrent file.metainfo
~~~

## Trivial Torrent Server

Implement the server part of the Trivial Torrent protocol. See Trivial Torrent protocol specification document for details (see section 2.3.2 of the specification document).

Use the file `src/ttorrent.c`.

The `ttorrent` command shall be extended to work as follows:

~~~{.diff}
$ bin/ttorrent -l 8080 file.metainfo
~~~

### Multi-client implementation strategies

The server part can be implemented using two different strategies: employing a *forking server*, or employing *non-blocking sockets*.
Employing non-blocking sockets can be *very* challenging. Follow one of the two approaches based on your personal preferences.
**Follow the conservative approach if you are not sure which approach is best for you.**

| Approach       | Description                                                                      |
| :-             | :------------                                                                    |
| *Conservative* | First implement the server employing a forking server model (mandatory; 70% grade), then upgrade the server to employ non-blocking sockets (optional; 30% grade). |
| *Audacious*    | Implement *directly* a server employing non-blocking sockets (100% grade).       |

Note: servers employing non-blocking sockets must employ a single process and must not employ threads.

## Practical Details

### Building

Use `make` in a terminal to build your project.

~~~{.bash}
$ make
~~~

This will create the executable file `bin/pong`.

### Reference binary

For your convenience, a reference binary is provided in the `reference_binary` directory. You can use this binary as a reference server while you develop your client.

### Testing

Some preliminary tests on the trivial torrent client can be performed as follows.

In one terminal run the reference server:

~~~{.bash}
$ reference_binary/ttorrent -l 8080 torrent_samples/server/test_file_server.ttorrent
~~~

In another terminal employ your client to download the file that the server is making available.

~~~{.bash}
$ bin/ttorrent torrent_samples/client/test_file.ttorrent
~~~

You can compare the original and the downloaded files using `cmp` to make sure they are equal.

~~~{.bash}
$ cmp torrent_samples/client/test_file.ttorrent torrent_samples/server/test_file_server.ttorrent
~~~

Note: this command does not output anything if both files are equal.

Additional tests may be run with:

~~~{.bash}
$ make test
~~~

Note that github runs `make test` for you every time you push something to this repository. You can see the results in the `Actions` tab.


