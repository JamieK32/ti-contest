import pyb
import sensor
import math
import ujson
import utime

uart = pyb.UART(1, 460800)

question0_count = 0
question2_idx = 0
question0_flag = True

rectangle_1 = [
    [109, 40],
    [240, 43],
    [241, 174],
    [109, 176],
]


def find_laser_point(img,  roi=None, red_threshold=(20, 100, 15, 127, -128, 30)):
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
    step = 20
    path_points = []

    for i in range(4):
        start, end = points[i], points[(i + 1) % 4]
        img.draw_line((start[0], start[1], end[0], end[1]), color=color)
        path_points += generate_points_on_edge(start, end, step)

    return path_points  # 不需要额外的收尾检查

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

def pack_data(x, y):
    data = {"x": x, "y": y}
    return ujson.dumps(data)

def process_question0(img):
    """处理 QUESTION0 逻辑 - 只执行一次"""
    global question0_count, question0_flag
    if question0_flag == False:
        return

    point = find_laser_point(img)
    if point[0] != -1 and point[1] != -1:  # 确保找到了激光点
        rectangle_1[question0_count] = point
        print(f"存储第{question0_count}个点: {point}")
        question0_count += 1

        if question0_count >= 4:
            question0_flag = False
            print("已收集完4个角点")

def process_question1(img):
    """处理 QUESTION1 逻辑"""
    global rectangle_1
    draw_rectangle_with_path(img, rectangle_1)
    center = get_rectangle_center(rectangle_1)
    cx, cy = center
    x, y = find_laser_point(img)
    draw_point(img, center)
    if x != -1 and y != -1:
        draw_point(img, (x, y), color=(0, 0, 0))
        err_x = cx - x
        err_y = cy - y
        json_data = pack_data(err_x, err_y)
        uart.write(json_data)

last_update_time = 0
UPDATE_INTERVAL = 350  # 100ms更新间隔

def process_question2(img):
    """处理 QUESTION2 逻辑 - 每100ms强制更新路径点"""
    global question2_idx, rectangle_1, last_update_time

    current_time = utime.ticks_ms()

    # 绘制路径
    path_points = draw_rectangle_with_path(img, rectangle_1)

    # 检查是否到达路径终点
    if question2_idx >= len(path_points):
        json_data = pack_data(0, 0)
        uart.write(json_data)
        return

    # 获取当前目标点
    cx = path_points[question2_idx][0]
    cy = path_points[question2_idx][1]

    # 查找激光点
    x, y = find_laser_point(img)
    if x != -1 and y != -1:
        draw_point(img, (x, y))
        err_x = cx - x
        err_y = cy - y
    else:
        return

    # 发送当前误差
    json_data = pack_data(err_x, err_y)
    uart.write(json_data)

    # 时间判断：每100ms强制更新索引
    if utime.ticks_diff(current_time, last_update_time) >= UPDATE_INTERVAL:
        question2_idx += 1
        last_update_time = current_time


def process_question3(img):
    """处理 QUESTION3 逻辑"""


def default_process(img):
    """处理默认逻辑，保持循环运行"""

def process_reset(img):
    global question2_idx, question0_count, question0_flag, rectangle_1
    question2_idx = 0
    question0_count = 0
    question0_flag = True
    rectangle_1 = [
        [110, 42],
        [239, 43],
        [239, 176],
        [109, 175],
    ]
    print("重置成功")

if __name__ == "__main__":
    # 初始化摄像头
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QVGA)
    sensor.skip_frames(time=2000)

    current_command = "DEFAULT"

    command_handlers = {
        "QUESTION0": process_question0,
        "QUESTION1": process_question1,
        "QUESTION2": process_question2,
        "QUESTION3": process_question3,
        "DEFAULT":   default_process,
        "RESET":     process_reset
    }

    while True:
           img = sensor.snapshot()

           command = parse_uart_cmd(uart)

           # 只有接收到新命令时才处理
           if command:
               if command in command_handlers:
                   current_command = command

               if command == "QUESTION0":
                   process_question0(img)  # 只执行一次
               elif command == "QUESTION2":
                   question2_idx = 0
               elif command == "QUESTION3":
                   find_black_rects(img)
               elif command == "RESET":
                   process_reset(img)

           # 对于持续性任务（QUESTION1, QUESTION2）
           if current_command in ["QUESTION1", "QUESTION2"]:
               command_handlers[current_command](img)

           pyb.delay(9)
