#!/usr/bin/env python3
import serial
import threading
import time
import sys

class SerialTerminal:
    def __init__(self):
        self.ser = None
        self.running = False
        self.read_thread = None

    def connect(self, port, baudrate=9600, data_bits=8, stop_bits=1, parity='N'):
        try:
            self.ser = serial.Serial(
                port=port,
                baudrate=baudrate,
                bytesize=data_bits,
                stopbits=stop_bits,
                parity=parity,
                timeout=1
            )
            self.running = True
            self.read_thread = threading.Thread(target=self.read_from_port)
            self.read_thread.start()
            print(f"Connesso a {port} a {baudrate} baud")
        except Exception as e:
            print(f"Errore di connessione: {str(e)}")

    def disconnect(self):
        if self.ser and self.ser.is_open:
            self.running = False
            self.read_thread.join()
            self.ser.close()
            print("Disconnesso")

    def read_from_port(self):
        while self.running and self.ser and self.ser.is_open:
            try:
                if self.ser.in_waiting > 0:
                    data = self.ser.read(self.ser.in_waiting)
                    print(data.decode('utf-8', errors='replace'), end='', flush=True)
            except Exception as e:
                print(f"\nErrore di lettura: {str(e)}")
                self.running = False
            time.sleep(0.01)

    def send_data(self, data):
        if self.ser and self.ser.is_open:
            try:
                self.ser.write(data.encode('utf-8') + b'\r\n')
            except Exception as e:
                print(f"Errore di invio: {str(e)}")



def main():
    terminal = SerialTerminal()
    
    print("Terminale Seriale Python")
    print("Comandi speciali:")
    print("  /help - Mostra questo messaggio")
    print("  /on   - set RTS and DTR to high")
    print("  /off  - set RTS and DTR to low")
    print("  /connect [porta] [baudrate] - Connetti a una porta")
    print("  /disconnect - Disconnessione")
    print("  /exit - Esci dal programma")

    port = "/dev/ttyUSB0"
    baudrate = 115200
    terminal.disconnect()
    terminal.connect(port, baudrate)
    terminal.ser.setDTR(False) #TM   (lo)

    while True:
        try:
            user_input = input("> ").strip()
            
            if user_input.startswith('/'):
                cmd = user_input.split()
                
                if cmd[0] == '/exit':
                    terminal.disconnect()
                    break
                
                elif cmd[0] == '/help':
                    print("Comandi disponibili:")
                    print("  /help - Mostra questo messaggio")
                    print("  /on   - set RTS and DTR to high")
                    print("  /off  - set RTS and DTR to low")
                    print("  /connect [porta] [baudrate] - Connetti a una porta")
                    print("  /disconnect - Disconnessione")
                    print("  /exit - Esci dal programma")
                
                
                elif cmd[0] == '/connect':
                    if len(cmd) < 2:
                        print("Specificare una porta (es: COM3 o /dev/ttyUSB0)")
                        continue
                    
                    port = cmd[1]
                    baudrate = int(cmd[2]) if len(cmd) > 2 else 9600
                    terminal.disconnect()
                    terminal.connect(port, baudrate)
                
                elif cmd[0] == '/disconnect':
                    terminal.disconnect()

                elif cmd[0] == '/cmd':
                    pkt = 'UXTDWU' # UXTL16 UDLL48 UXTDWU
                    terminal.ser.setRTS(True) #RSTN (lo)
                    terminal.ser.setDTR(True) #TM   (lo)
                    time.sleep(0.1)
                    terminal.ser.flushOutput()
                    terminal.ser.flushInput()
                    time.sleep(0.1)
                    terminal.ser.setRTS(False) #RSTN (lo)
                    terminal.ser.setDTR(False) #TM   (lo)
                    terminal.ser.timeout = 0.04

                    while True:
                        sent = terminal.ser.write(pkt.encode())
                        read = terminal.ser.read(6)
                        print(read)
                        if read == b'cmd>>:' :
                            break
                        if read == b'fct>>:' :
                            fct_mode = True
                            break
                    terminal.ser.baudrate = 115200
                    terminal.ser.timeout = 0.2
                    terminal.ser.write(str.encode('rdrev+ '))
                    terminal.ser.timeout = 0.1
                    read = terminal.ser.read(26)
                    print(read)


                elif cmd[0] == '/off':

                    terminal.ser.setRTS(True) #RSTN (lo)
                    terminal.ser.setDTR(True) #TM   (lo)
                elif cmd[0] == '/on':

                    terminal.ser.setRTS(False) #RSTN (lo)
                    terminal.ser.setDTR(False) #TM   (lo)
                elif cmd[0] == '/*':

                    terminal.ser.setRTS(True) #RSTN (lo)
                    terminal.ser.setDTR(True) #TM   (lo)
                    time.sleep(0.1)

                    terminal.ser.setRTS(False) #RSTN (lo)
                    terminal.ser.setDTR(False) #TM   (lo)

                else:
                    print("Comando non riconosciuto")
            
            else:
                if terminal.ser and terminal.ser.is_open:
                    terminal.send_data(user_input)
                else:
                    print("Non connesso a nessuna porta seriale")
        
        except KeyboardInterrupt:
            terminal.disconnect()
            break
        except Exception as e:
            print(f"Errore: {str(e)}")

if __name__ == "__main__":
    main()