from pyb import Servo  # 导入舵机模块
import pyb            # 用于延时
import sensor
import display


class TwoAxisServoControl:
    def __init__(self, servo_x_pin=1, servo_y_pin=2, x_init=0, y_init=-10,
                 x_limit=(-30, 30), y_limit=(-30, 30)):
        """
        初始化舵机设置

        Parameters:
            servo_x_pin: int - X轴舵机（默认连接到 Servo(1)）
            servo_y_pin: int - Y轴舵机（默认连接到 Servo(2)）
            x_init: int - X轴舵机的初始角度
            y_init: int - Y轴舵机的初始角度
            x_limit: tuple - X轴舵机角度限制 (最小值, 最大值)
            y_limit: tuple - Y轴舵机角度限制 (最小值, 最大值)
        """
        # 初始化舵机对象
        self.servo_x = Servo(servo_x_pin)
        self.servo_y = Servo(servo_y_pin)

        # 舵机当前角度值
        self.servo_x_value = x_init
        self.servo_y_value = y_init

        # 舵机角度限制
        self.servo_x_limit = x_limit
        self.servo_y_limit = y_limit

        # 设置初始位置
        self.servo_x.angle(self.servo_x_value)
        self.servo_y.angle(self.servo_y_value)

    def move_increment(self, x_increment, y_increment):
        """
        增量式移动舵机

        Parameters:
            x_increment: float - X轴增量（正值向右，负值向左）
            y_increment: float - Y轴增量（正值向上，负值向下）
        """
        # 计算新的角度值
        new_x = self.servo_x_value + x_increment
        new_y = self.servo_y_value + y_increment

        # X轴限位检查和移动
        if self.servo_x_limit[0] < new_x < self.servo_x_limit[1]:
            self.servo_x_value = new_x
            self.servo_x.angle(self.servo_x_value)

        # Y轴限位检查和移动
        if self.servo_y_limit[0] < new_y < self.servo_y_limit[1]:
            self.servo_y_value = new_y
            self.servo_y.angle(self.servo_y_value)

    def reset(self):
        """
        将舵机复位到初始位置
        """
        self.servo_x_value = 0
        self.servo_y_value = -10
        self.servo_x.angle(self.servo_x_value)
        self.servo_y.angle(self.servo_y_value)
        pyb.delay(100)  # 延时0.5秒以稳定

    def get_current_angles(self):
        """
        获取当前舵机角度
        """
        return self.servo_x_value, self.servo_y_value


class IncrementalPID:
    def __init__(self, kp, ki, kd, output_limits=(None, None)):
        """
        初始化增量式PID参数

        Parameters:
            kp: float - 比例增益
            ki: float - 积分增益
            kd: float - 微分增益
            output_limits: tuple - 输出限制 (最小值, 最大值)
        """
        self.kp = kp
        self.ki = ki
        self.kd = kd
        self.min_output, self.max_output = output_limits

        # 存储前两次误差
        self.prev_error = 0      # e(k-1)
        self.prev_prev_error = 0 # e(k-2)
        self.first_run = True
        self.second_run = True

    def compute(self, error):
        """
        计算增量式PID输出

        Parameters:
            error: float - 当前误差值

        Returns:
            float - PID控制增量输出
        """
        if self.first_run:
            # 第一次运行，只有比例项
            delta_output = self.kp * error
            self.first_run = False
        elif self.second_run:
            # 第二次运行，有比例项和积分项
            delta_output = self.kp * (error - self.prev_error) + self.ki * error
            self.second_run = False
        else:
            # 增量式PID公式：Δu(k) = Kp[e(k)-e(k-1)] + Ki*e(k) + Kd[e(k)-2e(k-1)+e(k-2)]
            delta_output = (self.kp * (error - self.prev_error) +
                           self.ki * error +
                           self.kd * (error - 2 * self.prev_error + self.prev_prev_error))

        # 更新误差历史
        self.prev_prev_error = self.prev_error
        self.prev_error = error

        # 输出限幅
        if self.min_output is not None and delta_output < self.min_output:
            delta_output = self.min_output
        if self.max_output is not None and delta_output > self.max_output:
            delta_output = self.max_output

        return delta_output

    def reset(self):
        """
        重置PID控制器状态
        """
        self.prev_error = 0
        self.prev_prev_error = 0
        self.first_run = True
        self.second_run = True

    def set_gains(self, kp=None, ki=None, kd=None):
        """
        更新PID增益
        """
        if kp is not None:
            self.kp = kp
        if ki is not None:
            self.ki = ki
        if kd is not None:
            self.kd = kd

def find_laser_point(img, red_threshold=(15, 75, 20, 90, 0, 60)):
    """
    寻找激光点位置
    """
    blobs = img.find_blobs([red_threshold], pixels_threshold=1, area_threshold=1, merge=True)
    if blobs:
        # 选择面积最大的色块
        largest_blob = max(blobs, key=lambda b: b.pixels())
        return largest_blob.cx(), largest_blob.cy()
    return None

def draw_centered_square(img, size, color=(255, 0, 0)):
    """
    在图像中心画一个正方形，并返回绕着正方形一圈的点位坐标（步进值20），最终回到原点
    """
    width = img.width()
    height = img.height()

    # 计算正方形的左上角坐标
    square_x = int((width - size) / 2)
    square_y = int((height - size) / 2)

    # 画中心正方形
    img.draw_rectangle((square_x, square_y, size, size), color=color)

    # 生成绕着正方形一圈的点位坐标，步进值为20
    points = []
    step = 10

    # 起始点（左上角）
    start_point = (square_x, square_y)
    points.append(start_point)

    # 上边：从左到右（跳过起始点）
    for x in range(square_x + step, square_x + size + 1, step):
        if x <= square_x + size:
            points.append((x, square_y))

    # 右边：从上到下（跳过右上角点）
    for y in range(square_y + step, square_y + size + 1, step):
        if y <= square_y + size:
            points.append((square_x + size, y))

    # 下边：从右到左（跳过右下角点）
    for x in range(square_x + size - step, square_x - 1, -step):
        if x >= square_x:
            points.append((x, square_y + size))

    # 左边：从下到上（跳过左下角和左上角点）
    for y in range(square_y + size - step, square_y, -step):
        if y > square_y:
            points.append((square_x, y))

    # 最终回到原点（起始点）
    points.append(start_point)

    return points


if __name__ == "__main__":

    # 初始化舵机控制器
    my_servo = TwoAxisServoControl()
    my_servo.reset()

    # 初始化摄像头
    sensor.reset()
    sensor.set_pixformat(sensor.RGB565)
    sensor.set_framesize(sensor.QQVGA2)
    sensor.skip_frames(time=2000)

    # 初始化LCD
    lcd = display.SPIDisplay()

    while True:
        img = sensor.snapshot()
        draw_centered_square(img, 100, color=(0, 255, 0))
        lcd.write(img)

    # 设置图像中心点为目标位置
    target_x = (128 - 100) / 2  # QVGA图像宽度320的一半
    target_y = (160 - 100) / 2  # QVGA图像高度240的一半

    # 初始化增量式PID控制器
    # 注意：增量式PID的参数调节相对更敏感，建议从小的值开始调试
    x_pid = IncrementalPID(
        kp=0.1,   # 比例增益，控制响应速度
        ki=0.015,  # 积分增益，消除稳态误差
        kd=0.00,  # 微分增益，减少超调
        output_limits=(-20, 20)  # 限制单次移动增量，避免舵机移动过大
    )

    y_pid = IncrementalPID(
        kp=0.1,
        ki=0.015,
        kd=0.00,
        output_limits=(-20, 20)
    )

    print("激光追踪系统启动...")
    print("目标位置: ({}, {})".format(target_x, target_y))

    last_time = 0
    idx = 0

    while True:
        img = sensor.snapshot()
        current_time = pyb.millis()


        # 绘制中心十字线
        points = draw_centered_square(img, 100, color=(0, 255, 0))

        # 寻找激光点
        laser_pos = find_laser_point(img)

        if laser_pos:
            laser_x, laser_y = laser_pos

            # 绘制激光点
            img.draw_circle(laser_x, laser_y, 8, color=(255, 255, 0))

            # 计算误差（目标位置 - 当前位置）
            error_x = target_x - laser_x
            error_y = target_y - laser_y

            # 计算PID增量输出
            x_increment = x_pid.compute(error_x)
            y_increment = y_pid.compute(error_y)

            # 增量式控制舵机移动
            # 注意：这里x_increment需要取反，因为舵机X轴移动方向与图像坐标可能相反
            my_servo.move_increment(-x_increment, -y_increment)


        else:
            # 如果没有检测到激光点，重置PID状态
            x_pid.reset()
            y_pid.reset()
            my_servo.reset()
            img.draw_string(10, 10, "No laser detected", color=(255, 0, 0))

        if current_time - last_time >= 200:
            last_time = current_time
            target_x, target_y = points[idx]
            idx = (idx + 1) % len(points)


        pyb.delay(10)  # 适当延时，避免控制频率过高


