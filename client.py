import os
import errno

class Packet:
    def __init__(self, buf=[]):
        if (len(buf) == 16):
            self.descriptor = int.from_bytes(buf[0:8], 'little')  
            self.buffer = int.from_bytes(buf[8:16], 'little') 
        else:
            self.descriptor = 0
            self.buffer = 0

    def to_buf(self):
        return self.descriptor.to_bytes(8, 'little') + self.buffer.to_bytes(8, 'little')

def safe_os_read(fd, count):
       try:
          return os.read(fd, count)
       except OSError as exc:
            if exc.errno == errno.EAGAIN:
                return [] 
            raise  ''' reads data from a pipe and returns `None` on EAGAIN '''


class FIFOInterface:
    def __init__(self, file, server=False):
        if server:
            os.mkfifo(file + "sread")
            os.mkfifo(file + "swrite")
        self.read_file = os.open(file + ("sread" if server else "swrite"), flags=(os.O_RDWR | os.O_NONBLOCK))
        self.write_file = os.open(file + ("swrite" if server else "sread"), flags=(os.O_RDWR | os.O_NONBLOCK))
        self.write_ready = server 
        self.server_client = server 

    def __del__(self):
        os.close(self.read_file)
        os.close(self.write_file)

    def read(self):
        self.buf = safe_os_read(self.read_file, 16) 
        if len(self.buf) != 16:
            return False
        return True

    def write(self, packet):
        self.write_buf(packet.to_buf())

    def write_buf(self, buf):
        if not self.write_ready:
            return

        os.write(self.write_file, buf)
        self.write_ready = False

    def get_packet(self):
        return Packet(self.buf)
