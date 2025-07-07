from maix import camera, display, image, uart, pinmap, time
from laser_detector import LaserDetector
from rectangle_utils import RectangleUtils
import json

device = "/dev/serial0"
serial0 = uart.UART(device, 115200)
cam = camera.Camera(480, 480)
disp = display.Display()
laser_dc = LaserDetector()
rt_util = RectangleUtils()

current_command = "RESET"
question0_count = 0
question0_flag = True
rectangle = [[153, 113], [441, 118], [443, 402], [155, 411]]
rectangle_points = []
rectangle_points_idx = 0
last_time = time.ticks_ms()  # 添加时间变量

def pack_data(x, y):
    data = {"x": x, "y": y}
    json_str = json.dumps(data)
    return json_str

def manual_marking(img):
    global question0_count, question0_flag
    if question0_flag == False:
        return
    
    point = laser_dc.detect(img)
    if point[0] != -1 and point[1] != -1:  # 确保找到了激光点
        rectangle[question0_count] = point
        print(f"存储第{question0_count}个点: {point}")
        
        # 绘制当前点
        img.draw_circle(point[0], point[1], 8, color=image.COLOR_GREEN, thickness=5)
        img.draw_string(point[0] + 10, point[1] - 10, f"P{question0_count}", 
                       color=image.COLOR_WHITE, scale=2)
        
        question0_count += 1
        if question0_count >= 4:
            question0_flag = False
            print("已收集完4个角点")

def process_reset_manual_marking(img):
    global question0_count, question0_flag
    question0_count = 0
    question0_flag = True

def process_track_center(img):
    """处理点追踪 - 计算当前激光与目标点的误差"""
    global rectangle

    # 绘制矩形和目标点
    rt_util.draw_rectangle(img, rectangle, color=image.COLOR_BLUE)
    target_point = rt_util.get_rectangle_center(rectangle)

    laser_result = laser_dc.detect(img)

    if laser_result[0] != -1 and laser_result[1] != -1:
        current_x, current_y = laser_result[0], laser_result[1]
        err_x = target_point[0] - current_x
        err_y = target_point[1] - current_y

        json_str = pack_data(err_x, err_y)
        serial0.write(json_str.encode('utf-8'))
        print(json_str)
    else:
        img.draw_string(10, 10, "No Laser!", color=image.COLOR_RED, scale=2)
        json_str = pack_data(0, 0)
        serial0.write(json_str.encode('utf-8'))

def process_track_path_point(img):
    """处理路径点追踪 - 沿矩形边缘移动"""
    global rectangle, rectangle_points_idx, rectangle_points, last_time
    
    # 绘制矩形
    rt_util.draw_rectangle(img, rectangle, color=image.COLOR_BLUE)
    
    # 生成矩形路径点
    rectangle_points = rt_util.generate_rectangle_points(rectangle)
    
    # 检查是否有路径点
    if not rectangle_points:
        json_str = pack_data(0, 0)
        serial0.write(json_str.encode('utf-8'))
        return
    
    # 确保索引在有效范围内（循环追踪）
    if rectangle_points_idx >= len(rectangle_points):
        rectangle_points_idx = 0
    
    # 获取当前目标点
    target_point = rectangle_points[rectangle_points_idx]
    
    # 检测激光点
    laser_result = laser_dc.detect(img)
    
    if laser_result[0] != -1 and laser_result[1] != -1:
        current_x, current_y = laser_result[0], laser_result[1]
        
        # 计算误差
        err_x = target_point[0] - current_x
        err_y = target_point[1] - current_y
        
        # 检查时间间隔和收敛情况，智能切换目标点
        current_time = time.ticks_ms()
        distance = ((err_x ** 2) + (err_y ** 2)) ** 0.5
        
        # 智能切换条件：时间到或者已收敛
        time_ready = current_time - last_time >= 600  # 延长到600ms
        converged = distance < 15  # 收敛阈值
        
        if time_ready or (converged and current_time - last_time >= 200):
            rectangle_points_idx += 1
            if rectangle_points_idx >= len(rectangle_points):
                rectangle_points_idx = 0  # 循环路径
            last_time = current_time
            print(f"切换目标点: 距离={distance:.1f}, 时间间隔={current_time-last_time}ms")
        
        json_str = pack_data(err_x, err_y)
        serial0.write(json_str.encode('utf-8'))
        print(f"Target: {target_point}, Current: ({current_x}, {current_y}), Error: ({err_x}, {err_y})")
        
        # 绘制当前激光点和目标点
        img.draw_circle(current_x, current_y, 5, color=image.COLOR_RED, thickness=3)
        img.draw_circle(target_point[0], target_point[1], 8, color=image.COLOR_GREEN, thickness=3)
        
    else:
        img.draw_string(10, 10, "No Laser!", color=image.COLOR_RED, scale=2)
        json_str = pack_data(0, 0)
        serial0.write(json_str.encode('utf-8'))

def process_stop(img):
    """停止处理"""
    json_str = pack_data(0, 0)
    serial0.write(json_str.encode('utf-8'))
    return 0

command_handlers = {
    "MANUAL_MARKING": manual_marking,
    "RESET_MANUAL_MARKING": process_reset_manual_marking,
    "TRACK_POINT_CENTER": process_track_center,
    "TRACK_PATH_POINT": process_track_path_point,
    "STOP": process_stop
}

def process_serial_data(data, img):
    """处理串口接收到的数据"""
    global current_command
    try:
        received_str = data.decode('utf-8').strip()
        print(f"received: '{received_str}'")
        
        if received_str in command_handlers:
            current_command = received_str
            if current_command in ["MANUAL_MARKING", "RESET_MANUAL_MARKING"]:
                command_handlers[current_command](img)
        else:
            print(f"未知命令: {received_str}")
            
    except UnicodeDecodeError:
        print(f"received (raw bytes): {data}")
        print(f"received (hex): {data.hex()}")

# 主循环
while True:
    img = cam.read()
    
    data = serial0.read()  
    
    if data: 
        process_serial_data(data, img)
    
    if current_command in ["TRACK_POINT_CENTER", "TRACK_PATH_POINT"]:
        command_handlers[current_command](img)
    
    disp.show(img)
    time.sleep_ms(15)