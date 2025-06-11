import tkinter as tk
from tkinter import ttk, messagebox
import serial
import serial.tools.list_ports
import json
import threading
import time
import os
import csv
from datetime import datetime

class SCPIMeterApp:
    def __init__(self, root):
        self.root = root
        self.root.title("ET44/ET45 LCR Meter Controller - https://github.com/semandko")
        self.root.geometry("650x600")

        self.ser = None
        self.commands = self.load_scpi_commands()

        # COM port selection
        port_frame = tk.Frame(root)
        port_frame.pack(pady=5)

        tk.Label(port_frame, text="Select COM Port:").pack(side=tk.LEFT)
        self.port_combobox = ttk.Combobox(port_frame, width=20, state="readonly")
        self.port_combobox.pack(side=tk.LEFT, padx=5)
        self.refresh_ports()

        refresh_btn = tk.Button(port_frame, text="Refresh", command=self.refresh_ports)
        refresh_btn.pack(side=tk.LEFT, padx=5)

        connect_btn = tk.Button(port_frame, text="Connect", command=self.connect_serial)
        connect_btn.pack(side=tk.LEFT, padx=5)

        self.status_label = tk.Label(root, text="Not Connected", fg="red")
        self.status_label.pack()

        # SCPI Command Section
        command_frame = tk.LabelFrame(root, text="SCPI Commands")
        command_frame.pack(fill="both", expand=True, padx=10, pady=10)

        tk.Label(command_frame, text="Select Command:").pack()
        self.command_var = tk.StringVar()
        self.command_combobox = ttk.Combobox(command_frame, textvariable=self.command_var, state="readonly", width=60)
        self.command_combobox.pack(pady=5)
        self.command_combobox['values'] = list(self.commands.keys())
        self.command_combobox.bind("<<ComboboxSelected>>", self.fill_command_entry)

        tk.Label(command_frame, text="SCPI Command:").pack()
        self.command_entry = tk.Entry(command_frame, width=60)
        self.command_entry.pack(pady=5)

        send_btn = tk.Button(command_frame, text="Send Command", command=self.send_command)
        send_btn.pack(pady=5)

        # Frequency sweep section
        sweep_frame = tk.LabelFrame(root, text="Frequency Sweep")
        sweep_frame.pack(fill="both", expand=True, padx=10, pady=10)

        self.range_entries = []
        self.default_steps = [(10, 100, 1), (100, 500, 2), (500, 1000, 5),
                               (1000, 5000, 10), (5000, 20000, 100), (20000, 100000, 500)]

        for start, stop, step in self.default_steps:
            frame = tk.Frame(sweep_frame)
            frame.pack(pady=2)
            tk.Label(frame, text=f"{start} - {stop} Hz Step:").pack(side=tk.LEFT)
            step_var = tk.StringVar(value=str(step))
            entry = tk.Entry(frame, textvariable=step_var, width=5)
            entry.pack(side=tk.LEFT)
            self.range_entries.append((start, stop, entry))

        sweep_btn = tk.Button(sweep_frame, text="Start Sweep & Log CSV", command=self.start_sweep_thread)
        sweep_btn.pack(pady=5)

        # Output section
        output_frame = tk.LabelFrame(root, text="Output")
        output_frame.pack(fill="both", expand=True, padx=10, pady=10)

        self.output_text = tk.Text(output_frame, height=15)
        self.output_text.pack(fill="both", expand=True)

        # Close serial on exit
        self.root.protocol("WM_DELETE_WINDOW", self.close_app)

    def load_scpi_commands(self):
        try:
            with open("scpi_commands.json", "r", encoding="utf-8") as f:
                return json.load(f)
        except Exception as e:
            messagebox.showerror("JSON Load Error", f"Error loading SCPI commands:\n{e}")
            return {}

    def refresh_ports(self):
        ports = [port.device for port in serial.tools.list_ports.comports()]
        self.port_combobox['values'] = ports
        if ports:
            self.port_combobox.current(0)

    def connect_serial(self):
        port = self.port_combobox.get()
        if not port:
            messagebox.showwarning("No Port Selected", "Please select a COM port.")
            return
        try:
            self.ser = serial.Serial(port, baudrate=9600, timeout=1)
            self.status_label.config(text=f"Connected to {port}", fg="green")
        except Exception as e:
            messagebox.showerror("Connection Error", str(e))

    def fill_command_entry(self, event=None):
        selected_cmd = self.command_var.get()
        self.command_entry.delete(0, tk.END)
        self.command_entry.insert(0, selected_cmd)

    def send_command(self):
        if not self.ser or not self.ser.is_open:
            messagebox.showwarning("Not Connected", "Please connect to the instrument first.")
            return
        cmd = self.command_entry.get().strip()
        if not cmd:
            messagebox.showwarning("Empty Command", "Please enter a SCPI command.")
            return
        try:
            self.ser.write((cmd + '\n').encode())
            if '?' in cmd:
                time.sleep(0.1)
                response = self.ser.readline().decode().strip()
                self.log_output(f"> {cmd}\n< {response}")
            else:
                self.log_output(f"> {cmd}")
        except Exception as e:
            self.log_output(f"Error sending command: {e}")

    def start_sweep_thread(self):
        thread = threading.Thread(target=self.sweep_frequency, daemon=True)
        thread.start()

    def sweep_frequency(self):
        if not self.ser or not self.ser.is_open:
            self.log_output("Connect to instrument before sweeping.")
            return

        self.send_raw("FUNCtion:IMPedance:A Z")
        self.send_raw("FUNCtion:IMPedance:B THR")
        self.send_raw("FUNCtion:IMPedance:EQUivalent PALlel")
        self.send_raw("FUNCtion:IMPedance:RANGe:AUTO ON")
        self.send_raw("APERture SLOW")

        os.makedirs("logs", exist_ok=True)
        log_filename = datetime.now().strftime("logs/measurement_log_%Y%m%d_%H%M%S.csv")

        try:
            with open(log_filename, mode='w', newline='') as csv_file:
                csv_writer = csv.writer(csv_file)
                csv_writer.writerow(["Frequency (Hz)", "Impedance (A)", "Phase/Parameter (B)", "Timestamp"])

                for start, stop, entry in self.range_entries:
                    try:
                        step = int(entry.get())
                        if step < 1:
                            step = 1
                    except ValueError:
                        step = 1

                    freq = start
                    while freq <= stop:
                        self.send_raw(f"FREQuency {freq}")
                        time.sleep(0.3)
                        self.send_raw("*TRG")
                        self.ser.write(b"FETCh?\n")
                        time.sleep(0.3)
                        response = self.ser.readline().decode().strip()
                        self.log_output(f"{freq} Hz -> {response}")

                        try:
                            measured = response.split(",")
                            A_val = float(measured[0])
                            B_val = float(measured[1])
                            timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                            csv_writer.writerow([freq, A_val, B_val, timestamp])
                        except Exception as e:
                            self.log_output(f"Parse error: {e}")

                        freq += step
        except Exception as e:
            self.log_output(f"Error during logging: {e}")

        self.log_output(f"Measurement finished. Log saved to {log_filename}")

    def send_raw(self, cmd):
        try:
            self.ser.write((cmd + '\n').encode())
        except Exception as e:
            self.log_output(f"Error sending: {cmd} -> {e}")

    def log_output(self, text):
        self.output_text.insert(tk.END, text + '\n')
        self.output_text.see(tk.END)

    def close_app(self):
        if self.ser and self.ser.is_open:
            self.ser.close()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = SCPIMeterApp(root)
    root.mainloop()
