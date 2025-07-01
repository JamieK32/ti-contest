import pyb
import sensor
import math
import ujson

uart = pyb.UART(1, 115200)

def find_laser_point(img,  roi=None, red_threshold=(20, 100, 10, 127, 0, 30)):
    """寻找激光点位置 - 支持ROI"""
    if roi:
        # roi格式: (x, y, width, height)
        blobs = img.find_blobs([red_threshold], roi=roi, pixels_threshold=1, area_threshold=1, merge=True)
    else:
        blobs = img.find_blobs([red_threshold], pixels_threshold=1, area_threshold=1, merge=True)

    if blobs:
        # 选择面积最大的色块
        largest_blob = max(blobs, key=lambda b: b.pixels())
        return largest_blob.cx(), largest_blob.cy()
    return -1, -1


def draw_rectangle_with_path(img, points, color=(255, 0, 0)):
    """绘制矩形，并返回沿矩形边的离散点路径"""
    step = 5
    path_points = []

    for i in range(4):
        start, end = points[i], points[(i + 1) % 4]
        img.draw_line((start[0], start[1], end[0], end[1]), color=color)
        path_points += generate_points_on_edge(start, end, step)

    # 确保路径首尾闭合
    if path_points and path_points[-1] != points[0]:
        path_points.append(points[0])

    return path_points

def generate_points_on_edge(start_point, end_point, step):
    """
    在两个点之间生成等间距的离散点

    参数:
    start_point: 起始点 (x, y)
    end_point: 结束点 (x, y)
    step: 步长间距

    返回:
    points: 该边上的离散点列表
    """
    x1, y1 = start_point
    x2, y2 = end_point

    # 计算两点间的距离
    distance = math.sqrt((x2 - x1)**2 + (y2 - y1)**2)

    # 如果距离太小，直接返回起始点和结束点
    if distance < step:
        return [start_point, end_point]

    # 计算需要的步数
    num_steps = int(distance / step)

    points = []

    # 生成中间点
    for i in range(num_steps + 1):
        t = i / num_steps if num_steps > 0 else 0
        x = x1 + t * (x2 - x1)
        y = y1 + t * (y2 - y1)
        points.append((int(round(x)), int(round(y))))

    # 确保最后一个点是准确的结束点
    if points[-1] != end_point:
        points.append(end_point)

    return points

def get_rectangle_center(rectangle_points):
    """根据对角线交点计算矩形中心点"""
    x1, y1 = rectangle_points[0]
    x2, y2 = rectangle_points[2]

    return (x1 + x2) // 2, (y1 + y2) // 2

def draw_point(img, point, color=(0, 255, 0), size=5):
    x, y = point
    # 画圆
    img.draw_circle(x, y, size, color=color, thickness=2)
    # 画十字
    img.draw_line((x - size, y, x + size, y), color=color, thickness=2)  # 横线
    img.draw_line((x, y - size, x, y + size), color=color, thickness=2)  # 竖线

def parse_uart_cmd(uart):
    """解析UART命令"""
    try:
        if uart.any():
            data = uart.readline().decode('utf-8').strip()
            return data
    except Exception as e:
        print("UART解析错误: ", e)
    return None

def get_default_rectangle():
    rectangle = (
        (99, 60),
        (222, 60),
        (221, 190),
        (95, 185),
    )
    return rectangle

def pack_data(x, y):
    data = {"x": x, "y": y}
    return ujson.dumps(data)

def process_question1(img):
    """处理 QUESTION1 逻辑"""

    rectangle = get_default_rectangle()
    draw_rectangle_with_path(img, rectangle)
    center = get_rectangle_center(rectangle)
    cx, cy = center
    roi = (cx-50, cy-50, 100, 100)
    x, y = find_laser_point(img, roi)
    draw_point(img, center)
    if x != -1 and y != -1:
        draw_point(img, (x, y), color=(255, 0, 0))
        err_x = cx - x
        err_y = cy - y
        json_data = pack_data(err_x, err_y)
        uart.write(json_data)
        print(json_data)
    else:
        print("未检测到激光点")


def process_question2(img):
    """处理 QUESTION2 逻辑"""


def process_question3(img):
    """处理 QUESTION3 逻辑"""


def default_process(img):
    """处理默认逻辑，保持循环运行"""
    pyb.delay(1000)
    print("Idle: Default running process...")

if __name__ == "__main__":
    # 初始化摄像头
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time=2000)
    # 当前指令状态
    current_command = "DEFAULT"
    # 命令处理字典
    command_handlers = {
        "QUESTION1": process_question1,
        "QUESTION2": process_question2,
        "QUESTION3": process_question3,
        "DEFAULT": default_process
    }
    while True:
        img = sensor.snapshot()

        command = parse_uart_cmd(uart)
        if command in command_handlers:
            current_command = command

        command_handlers.get(current_command, default_process)(img)

        pyb.delay(20)
