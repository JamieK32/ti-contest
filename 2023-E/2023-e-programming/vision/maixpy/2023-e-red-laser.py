from maix import camera, display, image, uart, pinmap
import math

def on_received(serial : uart.UART, data : bytes):
    try:
        # 转换为字符串并去除首尾空白字符
        received_str = data.decode('utf-8').strip()
        print(f"received: '{received_str}'")
        
        # 可选：回发确认
        # serial.write(data)
        # print(f"sent back: '{received_str}'")
        
    except UnicodeDecodeError:
        # 如果解码失败，显示原始字节
        print(f"received (raw bytes): {data}")
        print(f"received (hex): {data.hex()}")

device = "/dev/serial0"

serial0 = uart.UART(device, 115200)
serial0.set_received_callback(on_received)

ports = uart.list_devices()
cam = camera.Camera(480, 480)
disp = display.Display()

area_threshold = 4
pixels_threshold = 4
thresholds = [(0, 100, 37, 113, 0, 20)]

CENTER_X, CENTER_Y = 240, 240

rectangle_1 = [
    [118, 110],
    [412, 111],
    [415, 409],
    [118, 412],
]

def lightweight_filter(blobs):
    if not blobs:
        return None
    
    valid_blobs = [b for b in blobs if 5 <= b.area() <= 500]
    
    if not valid_blobs:
        return None
    
    best_blob = min(valid_blobs, 
                   key=lambda b: math.sqrt((b.cx()-CENTER_X)**2 + (b.cy()-CENTER_Y)**2))
    
    return best_blob

def draw_rectangle_with_path(img, points, color=image.COLOR_WHITE):
    """绘制矩形，并返回沿矩形边的离散点路径"""
    step = 20
    path_points = []

    for i in range(4):
        start, end = points[i], points[(i + 1) % 4]
        # 修复：正确的参数格式
        img.draw_line(start[0], start[1], end[0], end[1], color=color, thickness=2)
        path_points += generate_points_on_edge(start, end, step)

    return path_points

def generate_points_on_edge(start_point, end_point, step):
    """
    在两个点之间生成等间距、包含起点和终点的离散点列表（收尾相连）

    参数:
        start_point (tuple): 起始点 (x, y)
        end_point (tuple): 结束点 (x, y)
        step (float): 点之间的步长

    返回:
        List[tuple]: 离散点列表（包含起点和终点，且无重复）
    """
    x1, y1 = start_point
    x2, y2 = end_point

    # 计算两点之间的欧几里得距离
    distance = math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)

    # 距离太小直接返回两个端点
    if distance <= step:
        return [start_point, end_point] if start_point != end_point else [start_point]

    # 计算步数，确保包含最后一个点
    num_steps = int(distance / step)
    points = []

    for i in range(num_steps + 1):
        t = i / num_steps
        x = x1 + t * (x2 - x1)
        y = y1 + t * (y2 - y1)
        points.append((int(round(x)), int(round(y))))

    # 添加 end_point 做收尾连接（若不同于最后一个点）
    if points[-1] != end_point:
        points.append(end_point)

    # 去重（避免连续重复的点）
    result = []
    for pt in points:
        if not result or pt != result[-1]:
            result.append(pt)

    return result

def get_rectangle_center(rectangle_points):
    """根据对角线交点计算矩形中心点"""
    x1, y1 = rectangle_points[0]
    x2, y2 = rectangle_points[2]

    return (x1 + x2) // 2, (y1 + y2) // 2

while 1:
    img = cam.read()
    draw_rectangle_with_path(img, rectangle_1)
    blobs = img.find_blobs(thresholds,
                          area_threshold=area_threshold,
                          pixels_threshold=pixels_threshold)
    
    best_blob = lightweight_filter(blobs)
    
    if best_blob:
        raw_cx, raw_cy = best_blob.cx(), best_blob.cy()
        

        
        img.draw_cross(raw_cx, raw_cy, color=image.COLOR_RED, size=10, thickness=2)
        img.draw_circle(raw_cx, raw_cy, 5, color=image.COLOR_GREEN, thickness=2)
        
  
    
    disp.show(img)