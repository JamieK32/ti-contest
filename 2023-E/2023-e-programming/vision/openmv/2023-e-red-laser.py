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


def draw_rectangle_with_path(img, points, color=(0, 0, 0)):
    """绘制矩形，并返回沿矩形边的离散点路径"""
    step = 3
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



def find_black_rects(img, black_threshold=(0, 13, -128, 10, -25, 50)):
    """
    OpenMV矩形检测函数 - 使用颜色阈值
    """
    # 查找黑色矩形 - 添加颜色阈值
    rects = img.find_rects(thresholds=[black_threshold], threshold=10000)

    if not rects:
        print("没有找到任何黑色矩形!")
        return

    print(f"找到 {len(rects)} 个黑色矩形")

    for i, r in enumerate(rects):
        # 获取矩形信息
        x, y, w, h = r.rect()
        print(f"矩形{i}: x={x}, y={y}, w={w}, h={h}")


        # 获取四个角点
        pts = r.corners()
        print(f"矩形{i}角点: {pts}")

        # 绘制角点连线
        img.draw_line(pts[0][0], pts[0][1], pts[1][0], pts[1][1], color=(0, 0, 255), thickness=1)
        img.draw_line(pts[1][0], pts[1][1], pts[2][0], pts[2][1], color=(0, 0, 255), thickness=1)
        img.draw_line(pts[2][0], pts[2][1], pts[3][0], pts[3][1], color=(0, 0, 255), thickness=1)
        img.draw_line(pts[3][0], pts[3][1], pts[0][0], pts[0][1], color=(0, 0, 255), thickness=1)

        # 绘制角点圆圈
        for j, pt in enumerate(pts):
            img.draw_circle(pt[0], pt[1], 3, color=(0, 0, 255), thickness=2)
            print(f"  角点{j}: ({pt[0]}, {pt[1]})")

def draw_point(img, point, color=(0, 0, 0), size=5):
    x, y = point
    # 画圆
    img.draw_circle(x, y, size, color=color, thickness=2)


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
        (102, 26),
        (231, 28),
        (238, 163),
        (102, 166),
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
    x, y = find_laser_point(img)
    json_data1 = pack_data(x, y)
    print(json_data1)
    draw_point(img, center)
    if x != -1 and y != -1:
        draw_point(img, (x, y), color=(0, 0, 0))
        err_x = cx - x
        err_y = cy - y
        json_data = pack_data(err_x, err_y)
        uart.write(json_data)
        # print(json_data)
    else:
        print("未检测到激光点")

question2_idx = 0  # 全局变量定义

def process_question2(img):
    """处理 QUESTION2 逻辑 - 到达终点后完全停止"""
    global question2_idx  # 声明使用全局变量
    rectangle = get_default_rectangle()
    path_points = draw_rectangle_with_path(img, rectangle)

    # 如果已经完成了整个路径，发送停止信号
    if question2_idx >= len(path_points):
        # 发送停止信号
        json_data = pack_data(0, 0)
        print("路径完成，停止运动:", json_data)
        uart.write(json_data)
        pyb.delay(100)
        return  # 直接返回，不再执行后续逻辑

    # 获取当前目标点
    cx = path_points[question2_idx][0]
    cy = path_points[question2_idx][1]

    # 寻找激光点
    x, y = find_laser_point(img)

    if x != -1 and y != -1:
        draw_point(img, (x, y))
        err_x = cx - x
        err_y = cy - y
        json_data = pack_data(err_x, err_y)
        print(f"目标点 {question2_idx}: ({cx}, {cy}), 激光点: ({x}, {y}), 误差: {json_data}")
        uart.write(json_data)

        # 当激光点接近当前目标点时，移动到下一个目标点
        if abs(err_x) < 10 and abs(err_y) < 10:
            question2_idx += 1  # 简单地增加idx，在函数开头会检查是否超出范围



def process_question3(img):
    """处理 QUESTION3 逻辑"""


def default_process(img):
    """处理默认逻辑，保持循环运行"""



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
        if command == "QUESTION2":
            question2_idx = 0
        if command == "QUESTION3":
             find_black_rects(img)


        command_handlers.get(current_command, default_process)(img)

        pyb.delay(20)
