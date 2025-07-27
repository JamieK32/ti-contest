from maix import camera, image, display, app
from track import LineTracker
from uart import CamComm
from digit_recognition import ObjectDetector
from collections import deque


cam = camera.Camera(320, 240)
disp = display.Display()
tracker = LineTracker()
number = ObjectDetector()
comm = CamComm()

none_detect_count = 0
detect_count = 0
first_digit = None  
mode = None 

def detect_first_number(img):
    global first_digit
    number.detect_numbers(img)  
    if number.get_number():
        first_digit = number.get_number()
        print(f"首次检测数字: {first_digit}")
        comm.send_number(first_digit)  

def detect_track_data(img):
    trace = tracker.get_trace(img)
    trace_num = tracker.to_number(trace)
    tracker.draw(img, trace)
    comm.send_track(trace_num)  

def detect_number(img):
    global none_detect_count, detect_count  #
    
    number.detect_numbers(img)
    
    if number.get_result() is None:
        none_detect_count += 1
    elif number.get_number() == first_digit:  #
        detect_count += 1
        none_detect_count = 0  # 修复7: 检测到目标数字时重置none计数
    
    if detect_count >= 10:
        comm.send_command(number.get_position())
        detect_count = 0
        none_detect_count = 0  # 重置计数
        
    if none_detect_count >= 10:
        comm.send_command(0x03)
        detect_count = 0
        none_detect_count = 0  # 重置计数

commands = {
    "START": detect_first_number,
    "TRACK": detect_track_data,
    "NUMBER": detect_number,
}

while not app.need_exit():
    img = cam.read()
    result = comm.receive(timeout=100)
    
    if result and result in commands:
        mode = result
        commands[result](img)
    
    # 修复8: 添加持续执行当前模式
    if mode and mode != "STOP":
        commands[mode](img)
    
    disp.show(img)