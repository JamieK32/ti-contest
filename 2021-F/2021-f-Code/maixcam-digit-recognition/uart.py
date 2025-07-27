from maix import uart
class CamComm:
    def __init__(self, device="/dev/ttyS0", baudrate=115200):
        self.serial = uart.UART(device, baudrate)
        self.state, self.length, self.data, self.count = 0, 0, bytearray(), 0
    
    def _send(self, data):
        if isinstance(data, str):
            data = data.encode()
        packet = bytes([0xAA, len(data)]) + data + bytes([0x55])
        return self.serial.write(packet) > 0
    
    def _receive(self, timeout=2000):
        rx_data = self.serial.read(timeout=timeout)
        if not rx_data:
            return None
        
        for byte in rx_data:
            if self.state == 0 and byte == 0xAA:
                self.state, self.length, self.data, self.count = 1, 0, bytearray(), 0
            elif self.state == 1:
                self.length, self.state = byte, 2 if byte > 0 else 3
            elif self.state == 2:
                self.data.append(byte)
                self.count += 1
                if self.count >= self.length:
                    self.state = 3
            elif self.state == 3:
                self.state = 0
                if byte == 0x55:
                    return self.data.decode()
        return None
    
    def send_track(self, value):
        return self._send(f"T:0x{value:02X}")

    
    def send_number(self, value):
        """发送数字: N:123"""
        return self._send(f"N:{value}")
    
    def send_command(self, code):
        """发送命令: C:0xXX"""
        return self._send(f"C:0x{code:02X}")
    
    def receive(self, timeout=2000):
        """接收数据"""
        return self._receive(timeout)
    
    def send_receive(self, data, timeout=2000):
        """发送并接收"""
        return self._receive(timeout) if self._send(data) else None