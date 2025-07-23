from maix import camera, display, image, uart, pinmap, time
from laser_detector import LaserDetector
from rectangle_utils import RectangleUtils
from rectangle_detector import RectangleDetector
import json
import math

# 硬件初始化
device = "/dev/serial0"
serial0 = uart.UART(device, 115200)
cam = camera.Camera(480, 480)
disp = display.Display()
laser_dc = LaserDetector()
rt_util = RectangleUtils()
detector = RectangleDetector(cam, disp)

# 系统状态
current_mode = "IDLE"
mark_count = 0
mark_enabled = False
rectangle = [[97, 90], [392, 96], [398, 395], [92, 404]]
mark_rectangle = [[97, 90], [392, 96], [398, 395], [92, 404]]
path_points = []
path_index = 0
last_switch_time = time.ticks_ms()

# 追踪参数（可调整）
class TrackConfig:
    corner_threshold = 10.0      # 角点精度要求
    edge_threshold = 15.0        # 边缘精度要求
    edge_time_ms = 300          # 边缘切换时间
    corner_amplify = 1.2        # 角点误差放大
    step_size = 20              # 路径点间距
    
    # 更新模式选择（手动调整）
    # "MIXED" - 角点精度优先，边缘时间驱动（原来的模式）
    # "PRECISION" - 所有点都必须达到精度要求才切换
    update_mode = "MIXED"
    
# 手动切换更新模式示例：
# TrackConfig.update_mode = "PRECISION"  # 切换到精度模式
# TrackConfig.update_mode = "MIXED"      # 切换到混合模式

def send_error(err_x, err_y):
    """发送误差数据到STM32"""
    data = json.dumps({"x": err_x, "y": err_y})
    serial0.write(data.encode('utf-8'))

def is_corner(point, corners, tolerance=8.0):
    """检查点是否为角点"""
    px, py = point
    for cx, cy in corners:
        if math.sqrt((px-cx)**2 + (py-cy)**2) <= tolerance:
            return True
    return False

def handle_manual_mark(img):
    """手动标记角点"""
    global mark_count, mark_enabled, rectangle, mark_rectangle
    
    if not mark_enabled:
        return
    
    point = laser_dc.detect(img)
    if point[0] != -1 and point[1] != -1:
        mark_rectangle[mark_count] = point
        print(f"标记角点 {mark_count+1}: {point}")
        
        # 绘制标记点
        img.draw_circle(point[0], point[1], 8, color=image.COLOR_GREEN, thickness=5)
        img.draw_string(point[0] + 10, point[1] - 10, f"P{mark_count+1}", 
                       color=image.COLOR_WHITE, scale=2)
        
        mark_count += 1
        if mark_count >= 4:
            mark_enabled = False
            rectangle = mark_rectangle
            print("4个角点标记完成！")

def handle_track_center(img):
    """追踪矩形中心"""
    global rectangle
    
    # 绘制矩形
    rt_util.draw_rectangle(img, rectangle, color=image.COLOR_BLUE)
    center = rt_util.get_rectangle_center(rectangle)
    
    # 检测激光并计算误差
    laser_pos = laser_dc.detect(img)
    if laser_pos[0] != -1 and laser_pos[1] != -1:
        err_x = center[0] - laser_pos[0]
        err_y = center[1] - laser_pos[1]
        send_error(err_x, err_y)
        
        # 绘制状态
        img.draw_circle(laser_pos[0], laser_pos[1], 5, color=image.COLOR_RED, thickness=3)
        img.draw_circle(center[0], center[1], 8, color=image.COLOR_GREEN, thickness=3)
    else:
        img.draw_string(10, 10, "No Laser!", color=image.COLOR_RED, scale=2)
        send_error(0, 0)

def handle_track_path(img):
    """追踪矩形路径 - 支持多种更新模式"""
    global rectangle, path_points, path_index, last_switch_time
    
    # 生成路径点
    if not path_points:
        rt_util.set_step(TrackConfig.step_size)
        path_points = rt_util.generate_rectangle_points(rectangle)
    
    # 绘制矩形
    rt_util.draw_rectangle(img, rectangle, color=image.COLOR_BLUE)
    
    # 检查路径完成
    if path_index >= len(path_points):
        img.draw_string(10, 50, "Path Complete!", color=image.COLOR_GREEN, scale=2)
        send_error(0, 0)
        return
    
    # 获取当前目标
    target = path_points[path_index]
    is_corner_point = is_corner(target, rectangle)
    
    # 检测激光
    laser_pos = laser_dc.detect(img)
    if laser_pos[0] != -1 and laser_pos[1] != -1:
        # 计算误差
        err_x = target[0] - laser_pos[0]
        err_y = target[1] - laser_pos[1]
        distance = math.sqrt(err_x**2 + err_y**2)
        
        # 切换逻辑 - 根据更新模式选择
        current_time = time.ticks_ms()
        should_switch = False
        
        if TrackConfig.update_mode == "MIXED":
            # 混合模式：角点精度优先，边缘时间驱动
            if is_corner_point:
                # 角点：精度优先
                if distance < TrackConfig.corner_threshold:
                    should_switch = True
                    print(f"角点达标! 距离: {distance:.1f}")
                else:
                    img.draw_string(10, 70, f"Corner: {distance:.1f}", color=image.COLOR_YELLOW, scale=1)
            else:
                # 边缘：时间驱动
                if current_time - last_switch_time >= TrackConfig.edge_time_ms:
                    should_switch = True
                    print(f"边缘超时切换，距离: {distance:.1f}")
        
        elif TrackConfig.update_mode == "PRECISION":
            # 精度模式：所有点都必须达到精度要求
            threshold = TrackConfig.corner_threshold if is_corner_point else TrackConfig.edge_threshold
            if distance < threshold:
                should_switch = True
                point_type = "角点" if is_corner_point else "边缘"
                print(f"{point_type}达标! 距离: {distance:.1f}, 阈值: {threshold:.1f}")
            else:
                point_type = "Corner" if is_corner_point else "Edge"
                img.draw_string(10, 70, f"{point_type}: {distance:.1f}/{threshold:.1f}", 
                               color=image.COLOR_YELLOW, scale=1)
        
        # 执行切换
        if should_switch:
            path_index += 1
            last_switch_time = current_time
            print(f"切换到目标 {path_index}")
        
        # 角点误差放大（仅对角点生效）
        if is_corner_point:
            err_x *= TrackConfig.corner_amplify
            err_y *= TrackConfig.corner_amplify
        
        # 发送控制信号
        send_error(err_x, err_y)
        
        # 绘制状态
        img.draw_circle(laser_pos[0], laser_pos[1], 5, color=image.COLOR_RED, thickness=3)
        target_color = image.COLOR_ORANGE if is_corner_point else image.COLOR_GREEN
        img.draw_circle(target[0], target[1], 8, color=target_color, thickness=3)
        
        if is_corner_point:
            img.draw_string(target[0] + 15, target[1] - 20, "CORNER", color=image.COLOR_ORANGE, scale=1)
        
        # 显示当前模式
        mode_text = f"Mode: {TrackConfig.update_mode}"
        img.draw_string(10, 90, mode_text, color=image.COLOR_WHITE, scale=1)
        
    else:
        img.draw_string(10, 10, "No Laser!", color=image.COLOR_RED, scale=2)
        send_error(0, 0)

# 命令处理函数
def cmd_start_marking():
    """开始手动标记"""
    global current_mode, mark_count, mark_enabled, rectangle
    print("开始手动标记模式")
    current_mode = "MANUAL_MARK"
    mark_count = 0
    mark_enabled = True

def cmd_reset_marks():
    """重置标记"""
    global mark_count, mark_enabled, rectangle
    print("重置标记")
    mark_count = 0
    mark_enabled = True
    rectangle = [[97, 90], [392, 96], [398, 395], [92, 404]]

def cmd_track_center():
    """开始中心追踪"""
    global current_mode
    print("开始中心追踪")
    current_mode = "TRACK_CENTER"

def cmd_track_path():
    """开始路径追踪"""
    global current_mode, path_points, path_index, rectangle
    rectangle = mark_rectangle
    print("开始路径追踪")
    current_mode = "TRACK_PATH"
    path_points = []
    path_index = 0
    TrackConfig.corner_threshold = 12.0
    TrackConfig.edge_threshold = 15.0
    TrackConfig.edge_time_ms = 300
    TrackConfig.corner_amplify = 1.2
    rt_util.set_step(20)
    TrackConfig.update_mode = "MIXED"

def cmd_auto_detect():
    """自动检测矩形并开始追踪"""
    global current_mode, rectangle, path_points, path_index
    print("自动检测模式")
    rectangle = detector.detect_once()
    current_mode = "TRACK_PATH"
    path_points = []
    path_index = 0
    # 自动模式使用更保守的参数
    TrackConfig.corner_threshold = 12.0
    TrackConfig.edge_threshold = 10.0
    TrackConfig.edge_time_ms = 500
    TrackConfig.corner_amplify = 1.1
    rt_util.set_step(1)
    TrackConfig.update_mode = "PRECISION"

# 命令映射表（原版）
commands = {
    "MANUAL_MARKING": cmd_start_marking,
    "RESET_MANUAL_MARK": cmd_reset_marks,
    "TRACK_POINT_CENTER": cmd_track_center,
    "TRACK_PATH_POINT": cmd_track_path,
    "DETECT_RECTANGLE": cmd_auto_detect,
}

# 模式处理映射
mode_handlers = {
    "MANUAL_MARK": handle_manual_mark,
    "TRACK_CENTER": handle_track_center,
    "TRACK_PATH": handle_track_path
}

def process_serial_command(data):
    """处理串口命令"""
    try:
        cmd = data.decode('utf-8').strip()
        print(f"收到命令: {cmd}")
        
        if cmd in commands:
            commands[cmd]()
        else:
            print(f"未知命令: {cmd}")
            
    except UnicodeDecodeError:
        print(f"解码错误: {data.hex()}")

# 主循环（简化）
print("激光追踪系统启动...")

while True:
    img = cam.read()
    # 处理串口命令
    if serial0.available():
        data = serial0.read()
        if data:
            process_serial_command(data)
    
    # 执行当前模式
    if current_mode in mode_handlers:
        mode_handlers[current_mode](img)
    
    # 显示状态信息
    img.draw_string(10, 450, f"Mode: {current_mode}", color=image.COLOR_WHITE, scale=1)
    
    disp.show(img)
    time.sleep_ms(15)